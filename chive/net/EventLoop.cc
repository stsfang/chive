#include "chive/net/EventLoop.h"
// #include "chive/net/Channel.h"
#include "chive/net/poller/EPollPoller.h"
#include "chive/base/clog/chiveLog.h"

#include <sys/poll.h>
#include <iostream>
#include <cassert>
#include <sys/eventfd.h>
#include <unistd.h>
#include <signal.h>
#include <algorithm>

using namespace chive;
using namespace chive::net;

// 局部线程存储
__thread EventLoop* t_loopInThisThread = nullptr;
//
const int kPollTimeMs = 10000;

/**
 * 创建一个eventfd并返回
 */
int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        CHIVE_LOG_DEBUG("Abort since creating eventfd failed! evtfd %d", evtfd);
        abort();    //中断程序
    }
    return evtfd;
}


//静态方法
EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

// 注意初始化的顺序与声明的顺序要一致
EventLoop::EventLoop():
    looping_(false),
    quit_(false),
    threadId_(CurrentThread::tid()),
    poller_(new EPollPoller(this)),
    callingPendingFunctors_(false),
    wakeupFd_(createEventfd()),
    wakeupChannel_(new Channel(this, wakeupFd_)),
    timerQueue_(new TimerQueue(this))
{
    CHIVE_LOG_DEBUG("EventLoop created %p in thread %d", this, threadId_);
    if(t_loopInThisThread)
    {
        CHIVE_LOG_ERROR("Another EventLoop %p exits in this thread %d",
                            t_loopInThisThread, threadId_);
    }
    else 
    {
        t_loopInThisThread = this;
    }

    //设置wakeupfd的回调函数，wakeupfd上有可读事件时调用
    wakeupChannel_->setReadCallback(
        std::bind(&EventLoop::handleRead, this));
    // 将可读event注册到poller
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    CHIVE_LOG_DEBUG("EventLoop %p of thread %d destructs in thread %d", 
                        this, threadId_, CurrentThread::tid());
    
    assert(!looping_);
    t_loopInThisThread = nullptr;

    // 移除 eventf并关闭
    //wakeupChannel_->disableAll();
    //wakeupChannel_->remove();
    ::close(wakeupFd_);
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    CHIVE_LOG_DEBUG("EventLoop %p start looping", this);

    while(!quit_)
    {
        activeChannels_.clear();
        poller_->poll(kPollTimeMs, &activeChannels_);
        CHIVE_LOG_DEBUG("trace in loop");
        for(ChannelList::iterator it = activeChannels_.begin();
            it != activeChannels_.end(); it++)
        {
            (*it)->handleEvent();
        }
        // 每次从poll中监听到事件时，检查是否有pendingFunctors待处理
        doPendingFunctors();
    }
    CHIVE_LOG_DEBUG("EventLoop %p stop looping", this);
    looping_ = false;
}

void EventLoop::updateChannel(Channel* channel)
{
    assert(channel->getOwnerLoop() == this);
    assertInLoopThread();   
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();

    // FIXME: poller未实现
    // poller_->removeChannel(channel);
    CHIVE_LOG_DEBUG("poller has not implement removeChannel yet");
}

//FIXME: 是否中断程序执行？
void EventLoop::abortNotInLoopThread() 
{
    CHIVE_LOG_ERROR("Error:EventLoop %p was created in threadId_ %d current thread %d", 
                            this, threadId_, CurrentThread::tid());
}

void EventLoop::quit()
{
    quit_ = true;
    // 
    // if(!isInLoopThread())
    // {
    //     wakeup();
    // }
}

TimerId EventLoop::runAt(Timer::Timestamp timeout, const Timer::TimerCallback& cb)
{
    return timerQueue_->addTimer(std::move(cb), timeout, 0);
}

TimerId EventLoop::runAfter(Timer::TimeType delay, const Timer::TimerCallback& cb)
{
    ///FIXME: static_cast是否多余
    ///FIXME: 传入的delay应该是微秒，还是秒
    return runAt(static_cast<Timer::Timestamp>(Timer::now() + delay), std::move(cb));
}

TimerId EventLoop::runEvery(Timer::TimeType interval, const Timer::TimerCallback& cb)
{
    return timerQueue_->addTimer(std::move(cb), Timer::now() + interval, interval);
}

void EventLoop::cancel(TimerId timerId)
{
    return timerQueue_->cancel(timerId);
}


void EventLoop::runInLoop(const Functor& cb)
{
    CHIVE_LOG_DEBUG("is loop thread %d",isInLoopThread());
    // 如果是在 IO 线程，那么直接执行 cb
    if(isInLoopThread())
    {
        CHIVE_LOG_DEBUG("in loop thread, callback now");
        cb();
    }
    else    // 否则放到 pendingFunctors_，唤醒 IO 线程去处理cb
    {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Functor& cb)
{
    // 需要加锁保护临界区，因为queueInLoop在非 IO 线程上被调用
    // pendingFunctors_ 同时可被 IO 线程访问
    CHIVE_LOG_DEBUG("trace in EventLoop::queueInLoop()");
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.push_back(cb);
    }

    // 如果不是在 IO 线程 或者 正在处理pending functors
    // 需要执行唤醒操作wakeup，唤醒 IO 线程处理wakeup fd

    // 为什么callingPendingFunctos_ = true也要执行wakeup()操作？
    // 见EventLoop::doPendingFunctors()
    // pendung functor 可能再调用 queuInLoop(cb2)，为了让cb2能被及时执行
    // 所以callingPendingFunctors为true的时候也尝试唤醒
    if(!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}

void EventLoop::doPendingFunctors() 
{
    CHIVE_LOG_DEBUG("trace in EventLoop::doPendingFunctors");
    std::vector<Functor> functors;
    callingPendingFunctors_ = true; //此时在IO线程，标志位不需要加锁保护

    // 同步，pendingFunctors_ 是共享对象
    {
        MutexLockGuard lock(mutex_);
        // 用局部对象换取共享对象，防止其他线程阻塞在等待 pendingFunctors_ 上
        // 减小临界区的长度，同时避免了死锁 (functor可能再调用queueInLoop() ) 
        functors.swap(pendingFunctors_);
    }

    CHIVE_LOG_DEBUG("functors size %d", functors.size());
    for (size_t i = 0; i < functors.size(); ++i)
    {
        functors[i]();
    }
    callingPendingFunctors_ = false; // 处理完pending task，重新置位
}

void EventLoop::wakeup()
{
    //向wakeupFd_写入8个字节，让poller收到wakeupFd_上有可读事件
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        CHIVE_LOG_ERROR("writes %d bytes instead of 8", n); 
    }
    CHIVE_LOG_INFO("write %d bytes", sizeof(one));
}

/**
 * wakeupFd_可读事件到来时的回调函数
 */
void EventLoop::handleRead()
{
    // 写入时写入8字节, 读出时一次读出8字节,表示一次通信
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        CHIVE_LOG_ERROR("reads %d bytes instead of 8", n);
                                
    }
}


