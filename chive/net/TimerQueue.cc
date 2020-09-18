#include "chive/net/TimerQueue.h"
#include "chive/base/Logger.h"
#include "chive/net/EventLoop.h"

#include "chive/net/Timer.h"

#include <algorithm>
#include <sys/timerfd.h>
#include <unistd.h>
#include <cassert>
#include <functional>
#include <utility>
#include <string.h> // memset

using namespace chive;
using namespace chive::net;

// timerfd 操作集合
namespace timerfdOps
{
timespec howMuchTimeFromNow(Timer::Timestamp when);
/**
 * 创建一个tiemrfd
 */
int createTimerfd()
{
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                 TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0)
  {
    debug() << "Failed in timerfd_create" << std::endl;
  }
  return timerfd;
}

/*
与timerfd_settime()有关的两个结构体
struct timespec {
    time_t tv_sec;      // Seconds 
    long   tv_nsec;     // Nanoseconds
};
struct itimerspec {
    struct timespec it_interval;  // Interval for periodic timer
    struct timespec it_value;     // Initial expiration 
};
*/
/**
 * 重置timerfd超时时间，仅当插入的定时器是最早到期的定时器
 * 如果不重置将错过最早的定时器
 */
void resetTimerfd(int timerfd, Timer::Timestamp expiration) 
{
    debug() << "trace in reset timerfd" << std::endl;
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&oldValue, 0, sizeof(oldValue));
    memset(&newValue, 0, sizeof(newValue));

    newValue.it_value = howMuchTimeFromNow(expiration);
    // expiration可能已经过期，需要重新设置新的超时时间
    int ret = timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if(ret) {
        debug(LogLevel::ERROR) << "timerfd_setttime()" << std::endl;
    }
}

timespec howMuchTimeFromNow(Timer::Timestamp when)
{
    Timer::Timestamp microseconds = when - Timer::now();
    if(microseconds < 100) {
        microseconds = 100;
    }
    timespec ts{};
    ts.tv_sec = static_cast<time_t>(microseconds / Timer::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>((microseconds % Timer::kMicroSecondsPerSecond) * 1000);

    return ts;
}

void readTimerfd(int timerfd, Timer::Timestamp now)
{
    uint64_t howmany = 1;
    
    ssize_t n = read(timerfd, &howmany, sizeof(howmany));
    debug() << "TimerQueue::readTimerfd() read " 
            << n << " bytes from timerfd " << timerfd 
            << " at " << now << std::endl;
    if(n != sizeof(howmany)) {
        debug(LogLevel::ERROR) << "TimerQueue::readTimerfd() reads "
                                << n << " bytes instead of 8 " 
                                << std::endl;
    }
}
};

TimerQueue::TimerQueue(EventLoop* loop)
    :loop_(loop),
     timerfd_(timerfdOps::createTimerfd()),
     timerfdChannel_(loop, timerfd_),
     timers_(),
     callingExpiredTimers_(false)
{
    // 设置"读"回调函数并开启可读 (更新到epoll)
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_.enableReading();
}  

TimerQueue::~TimerQueue()
{
    ::close(timerfd_);      // 关闭fd
}

TimerId TimerQueue::addTimer(const Timer::TimerCallback& cb, Timer::Timestamp when, Timer::TimeType interval) {
    std::shared_ptr<Timer> timer(new Timer(cb, when, interval));
    // 将添加定时器的操作转移到 IO 线程，让 IO 线程执行添加操作
    // 如此不加锁也能保证线程安全
    debug() << "trace in TimerQueue::addTimer()" << std::endl;
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return *(new TimerId(std::weak_ptr<Timer>(timer)));
}

void TimerQueue::cancel(const TimerId& timerId)
{
    // 注意到timer_成员是weak_ptr所以需要先lock检查
    // 引用对象是否还存在 
    auto timer = timerId.timer_.lock();
    if(timer) {
        loop_->runInLoop(
            std::bind(&TimerQueue::cancelInLoop, this, timerId));
    }
}

// FIXME: timer传参会不会导致引用计数+1
void TimerQueue::addTimerInLoop(const std::shared_ptr<Timer>& timer)
{
    debug() << "trace in TimerQueue::addTimerInLoop()" << std::endl;
    loop_->assertInLoopThread();
    bool isEarliest = insert(timer);
    
    if(isEarliest) {
        debug() << "trace in TimerQueue::addTimerInLoop() timestamp "
                << timer->getExpiredTime() << std::endl;
        timerfdOps::resetTimerfd(timerfd_, timer->getExpiredTime());
    }
}

void TimerQueue::cancelInLoop(const TimerId& timerId)
{
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    // TimeQueue是TimerId的友元类，可以直接访问private成员
    auto cancelTimer = timerId.timer_.lock();
    assert(cancelTimer);        // 检查弱指针的对象引用是都还存在
    ActiveTimer timer(cancelTimer, cancelTimer->getSequence());
    ActiveTimerSet::iterator it = activeTimers_.find(timer);
    if (it != activeTimers_.end())
    {
        // 同时删除两个容器中的定时器
        auto n = timers_.erase(Entry(it->first->getExpiredTime(), it->first));
        assert(n == 1); (void)n;
        activeTimers_.erase(it);
    }
    else if (callingExpiredTimers_)
    {
        // 防止定时器自注销
        cancelingTimers_.insert(timer);
    }
    // 检查删除的完整性
    assert(timers_.size() == activeTimers_.size());
}


bool TimerQueue::insert(const std::shared_ptr<Timer>& timer)
{
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    bool earliestChanged = false;
    Timer::Timestamp when = timer->getExpiredTime();
    // 如果插入的timer是第一个，或者是目前timestamp最早的一个
    // 那么earliestChanged为true
    auto it = timers_.begin();
    if(it == timers_.end() || when < it->first) {
        earliestChanged = true;
    }
    {
        auto result = timers_.insert(Entry(when, timer));
        assert(result.second); (void)result;
    }
    {
        auto result = activeTimers_.insert(ActiveTimer(timer, timer->getSequence()));
        assert(result.second); (void)result;
    }
    debug() << "trace in TimerQueue::insert() timestamp " << when 
            << " timer sequence " << timer->getSequence() << std::endl;
    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timer::Timestamp now) {
    std::vector<Entry> expired;

    Entry sentry = std::make_pair(now, std::shared_ptr<Timer>());
    auto it = timers_.lower_bound(sentry);

    assert(it == timers_.end() || now < it->first);

    std::copy(timers_.begin(), it, std::back_inserter(expired));
    timers_.erase(timers_.begin(), it);

    // 
    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timer::Timestamp now)
{
    for (const auto& it : expired)
    {
        // ActiveTimer: std::pair<std::shared_ptr<Timer>, int64_t>
        ActiveTimer timer(it.second, it.second->getSequence());
        // 如果timer是周期性触发的，那么需要重新启动
        if (it.second->repeat()
                && cancelingTimers_.find(timer) == cancelingTimers_.end())
        {
            it.second->restart(now);
            insert(it.second);
        }
        // else Timer is wrapper by std::shared_ptr, it will destructes itself
    }

    // 选择下一个timer的到期时间作为下一个时间戳
    // 检查该timer是否合法
    if(!timers_.empty())
    {
        auto nextTimer = timers_.begin()->second;   // timer is type std::shared_ptr
        if(nextTimer->isValid())
        {
            timerfdOps::resetTimerfd(timerfd_, nextTimer->getExpiredTime());
        }
    }
}

void TimerQueue::handleRead()
{
    loop_->assertInLoopThread();
    Timer::Timestamp now = Timer::now();
    timerfdOps::readTimerfd(timerfd_, now);
    std::vector<Entry> expiredTimers = getExpired(now);
    // 标志位，正在处理定时器任务
    callingExpiredTimers_ = true;
    cancelingTimers_.clear();

    debug() << "trace in TimerQueue::handleRead() run task" << std::endl;
    // 逐个调用定时器上绑定的任务
    for(const auto& timer : expiredTimers)
    {
        timer.second->run();
    }
    callingExpiredTimers_ = false;
    
    //处理完定时器任务需要重置
    ///TODO:
    reset(expiredTimers, now);
}
