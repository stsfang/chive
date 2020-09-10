///
/// __STDC_LIMIT_MACROS and __STDC_CONSTANT_MACROS are a workaround to allow C++ programs 
/// to use stdint.h macros specified in the C99 standard that aren't in the C++ standard. 
/// The macros, such as UINT8_MAX, INT64_MIN, and INT32_C() may be defined already in C++ applications in other ways.
/// To allow the user to decide if they want the macros defined as C99 does, many implementations require that __STDC_LIMIT_MACROS 
/// and __STDC_CONSTANT_MACROS be defined before stdint.h is included.
/// 
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <sys/timerfd.h>
#include <unistd.h>

#include "chive/net/TimerQueue.h"
#include "chive/net/EventLoop.h"
#include "chive/net/Timer.h"
#include "chive/net/TimerId.h"

namespace chive
{
namespace net
{
namespace detail
{

int createTimerfd()
{

}

struct timespec howMuchTimeFromNow(Timestamp when)
{

}

void readTimerfd(int timerfd, Timestamp now)
{

}

void resetTimerfd(int timerfd, Timestamp expiration)
{

}


} // namespace detail
} // namespace net
} // namespace chive

using namespace chive;
using namespace chive::net;
using namespace chive::net::detail;

TimerQueue::TimerQueue(EventLoop* loop):
    loop_(loop),
    timerfd_(createTimerfd()),
    timerfdChannel_(loop, timerfd_),
    timers_(),
    callingExpiredTimers(false)
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
    timerfdChannel_.disableAll();
    tiemrfdChannel_.remove();
    ::close(tiemrfd_);
    // not need to delete timers 
    // since we replace raw pointer as std::unique_ptr<T>
    /*
    for(const Entry& timer : timers_)
    {
        delete tiemr.second;
    }
    */
}

TimerId TimerQueue::addTimer(TimerCallback cb, 
                            Timerstamp when, 
                            double interval)
{
    std::shared_ptr<Timer> timer = 
            std::make_shared<Timer>(std::move(cb), when, interval); //shared count +1
    loop_->runInLoop(
        std::bind(&TimerQueue::addTimerInLoop, this, timer)); //shared count +1
    return TimerId(timer, timer->sequence());   //shared count +1
}   // `timer` out of scope, shared count -1

void TimerQueue::cancel(TimerId timerId)
{
    loop_->runInLoop(
        std::bind(&TimeQueue::cancelInLoop, this, timerId));
}

void TimerQueue::addTimerInLoop(std::shared_ptr<Timer> timer)
{
    bool earliestChanged = insert(std::move(timer));
    if(earliestChanged)
    {
        resetTimerfd(timerfd_, timer->expiration());
    }
}

void TimerQueue::cancelInLoop(TimerId tiemrId)
{
    
    assert(tiemrs_.size() == activeTimers_.size());
    ActiveTimer timer(timerId.timer_, tiemrId.sequnce_);
    /// FIXME:
    /// here maybe should use c++14 heterogeneous comparison lookup since
    /// ActiveTimerSet has entry type of `std::pair<std::shared_ptr<Timer>, int64_t> `
    /// c++11 not support yet
    ActiveTimerSet::iterator it = activeTimers_.find(timer);
    if(it != activeTimers_.end())
    {
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        assert(n == 1); (void)n;
        //delete it->first; // no need
        activeTimers_.erase(it);
    }
    else if(callingExpiredTimers_)
    {
        cancelingTimers_.insert(timer);
    }
    assert(timers_size() == activeTimers_size());
}

void TimerQueue::handleRead()
{
    
}