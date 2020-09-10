#ifndef CHIVE_NET_TIMERQUEUE_H
#define CHIVE_NET_TIMERQUEUE_H

#include <set>
#include <vector>
#include <memory>
#include <atomic>

#include "chive/base/copyable.h"

namespace chive
{
namespace net
{
class TimerQueue : noncopyable
{
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId addTimer(TimerCallback cb, Timestamp when, double interval);

    void cancel(TimerId timerId);
private:
    using Entry = std::pair<Timestamp, std::shared_ptr<Timer>>;
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<std::shared_ptr<Timer>, int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;

    void addTimerInLoop(std::shared_ptr<Timer> timer);
    void cancelInLoop(TimerId timerId);
    
    // called when timer alarms
    void handleRead();
    // move out all expired timers
    std::vector<Entry> getExpired(Timestamp now);

    void reset(const stf::vector<Entry>& expired, Timestamp now);
    // insert a timer
    bool inset(std::shared_ptr<Timer> timer);

    EventLoop* loop_;
    const int Timerfd_;
    Channel timerfdChannel_;
    // timers is sorted by timestamp
    TimerList timers_;

    // for cancel()
    std::atomic<bool> callingExpiredTimers_; // canceling status
    ActiveTimerSet activeTimers_;
    ActiveTimerSet cancelingTimers_;
}
} // namespace net

} // namespace chive


#endif