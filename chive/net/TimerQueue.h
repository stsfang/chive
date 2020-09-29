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

    /**
     * 添加一个定时器
     * @param cb 定时器到期的回调函数
     * @param timeout 定时器的到期时间戳
     * @param intervel 定时器的周期时间间隔 (无周期触发则为0)
     * @return 返回唯一标识定时器的<Timer,sequenceId>
     */
    TimerId addTimer(const Timer::TimerCallback &cb, 
                    Timer::Timestamp timeout, 
                    Timer::TimeType interval);
    /**
     * 取消一个已添加的定时器
     * @param 唯一标识定时器的<Timer,sequenceId>
     */
    void cancel(const TimerId& timerId);
    
private:
    using Entry = std::pair<Timer::Timestamp, std::shared_ptr<Timer>>;
    using TimerList = std::set<Entry>;

    // add for cancel() --begin
    using ActiveTimer = std::pair<std::shared_ptr<Timer>, int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;
    // add for cancel() --end

    EventLoop* loop_;           //
    const int timerfd_;         // 定时器 fd
    Channel timerfdChannel_;    //定时器timerfd专用channel
    TimerList timers_;          //定时器列表

    // add for cancel() timer --begin
    ActiveTimerSet activeTimers_;
    ActiveTimerSet cancelingTimers_;    /// 防止timer"自注销",即在回调中被注销
    /// FIXME: 使用atomic??
    bool callingExpiredTimers_;         /// 防止timer"自注销"
    // add for cancel() timer --end

    /**
     * 定时器到期，处理timerfd可读事件
     */
    void handleRead();

    /**
     * 获取到期的定时器列表
     * @param 当前时间戳
     * @return 返回到期的定时器列表
     */
    std::vector<Entry> getExpired(Timer::Timestamp now);

    /**
     * 重置周期定时器的到期时间(续命),选择下一个到期时间更新timerfd
     * 移除到期的一次性定时器
     * @param expired 到期的定时器列表(包括周期定时器)
     * @param now 当前时间戳
     */
    void reset(const std::vector<Entry>& expired, Timer::Timestamp now);

    /**
     * 插入一个定时器
     * @param timer 定时器（智能指针）
     * @return 
     */
    bool insert(const std::shared_ptr<Timer>& timer);

    /**
     * 作为回调函数, 用于IO线程异步添加定时器
     * 由EventLoop调用doPendingFunctors()分发执行
     */
    void addTimerInLoop(const std::shared_ptr<Timer>& timer);

    /**
     * 在EventLoop中撤销一个定时器，由cancel()调用
     * @param tiemrId 标识定时器的<Timer, sequenceId>
     */
    void cancelInLoop(const TimerId& timerId);
};
} // namespace net

} // namespace chive

#endif