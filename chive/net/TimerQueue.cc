#include "chive/net/TimerQueue.h"

#include <algorithm>
#include <sys/timerfd.h>
#include <unistd.h>
#include <cassert>
#include <functional>
#include <utility>


using namespace chive;
using namespace chive::net;

TimerId TimerQueue::addTimer(const Timer::TimerCallback& cb, Timer::Timestamp when, Timer::TimeType interval) {
    
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

