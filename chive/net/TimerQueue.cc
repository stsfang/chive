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


using namespace chive;
using namespace chive::net;

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


TimerQueue::TimerQueue(EventLoop* loop)
    :loop_(loop),
     timerfd_(createTimerfd()),
     timerfdChannel_(loop, timerfd_),
     timers_(),
     callingExpiredTimers_(false)
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_.enableReading();
}  

TimerQueue::~TimerQueue()
{
    
}

TimerId TimerQueue::addTimer(const Timer::TimerCallback& cb, Timer::Timestamp when, Timer::TimeType interval) {
    std::shared_ptr<Timer> timer(new Timer(cb, when, interval));
    // 将添加定时器的操作转移到 IO 线程，让 IO 线程执行添加操作
    // 如此不加锁也能保证线程安全
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return *(new TimerId(std::weak_ptr<Timer>(timer)));
}

// FIXME: timer传参会不会导致引用计数+1
void TimerQueue::addTimerInLoop(const std::shared_ptr<Timer>& timer)
{
    loop_->assertInLoopThread();
    Timer::Timestamp expiredTime = timer->getExpiredTime();
    bool isEarliest = insert(timer);
    if(isEarliest) {
        (void)expiredTime;
        //TimerId::resetTimerfd(timerfd_, expiredTime);
    }
}

bool TimerQueue::insert(const std::shared_ptr<Timer>& timer)
{
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    return true;
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

void TimerQueue::handleRead()
{
//   loop_->assertInLoopThread();
//   Timestamp now(Timestamp::now());
//   readTimerfd(timerfd_, now);

//   std::vector<Entry> expired = getExpired(now);

//   callingExpiredTimers_ = true;
//   cancelingTimers_.clear();
//   // safe to callback outside critical section
//   for (const Entry& it : expired)
//   {
//     it.second->run();
//   }
//   callingExpiredTimers_ = false;

//   reset(expired, now);
}
