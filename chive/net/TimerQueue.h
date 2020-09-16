#ifndef CHIVE_NET_TIMERQUEUE_H
#define CHIVE_NET_TIMERQUEUE_H

#include <set>
#include <vector>
#include <memory>

#include "chive/base/noncopyable.h"
#include "chive/net/Channel.h"
#include "chive/net/Timer.h"
#include "chive/net/TimerId.h"


namespace chive
{
namespace net
{

class EventLoop;

class TimerQueue : noncopyable 
{
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId addTimer(const Timer::TimerCallback &cb, 
                    Timer::Timestamp timeout, 
                    Timer::TimeType interval);
    void cancel(TimerId timerId){}
    
private:
    using Entry = std::pair<Timer::Timestamp, std::shared_ptr<Timer>>;
    using TimerList = std::set<Entry>;

    using ActiveTimer = std::pair<std::shared_ptr<Timer>, int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;

    EventLoop* loop_;           //
    const int timerfd_;         // 定时器 fd
    Channel timerfdChannel_;    //定时器timerfd专用channel
    TimerList timers_;          //定时器列表

    ActiveTimerSet activeTimers_;
    ActiveTimerSet cancelingTimers_;
    bool callingExpiredTimers_;

    /**
     * 定时器到期，timerfd可读
     */
    void handleRead();

    /**
     * 移除到期的定时器
     * @param 当前时间戳
     * @return 返回到期被移除的定时器
     */
    std::vector<Entry> getExpired(Timer::Timestamp now);

    void reset(const std::vector<Entry>& expired, Timer::Timestamp now);

    /**
     * 插入一个定时器
     * @param timer 定时器（智能指针）
     * @return 
     */
    bool insert(const std::shared_ptr<Timer>& timer);
    void addTimerInLoop(const std::shared_ptr<Timer>& timer);

    
};
} // namespace net

} // namespace chive

#endif