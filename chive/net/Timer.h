#ifndef CHIVE_NET_TIMER_H
#define CHIVE_NET_TIMER_H

#include <cstdint>
#include <functional>

#include "chive/base/noncopyable.h"
#include "chive/base/Atomic.h"

namespace chive
{
namespace net
{

class Timer : noncopyable 
{
public:
    using TimerCallback = std::function<void()>;    // 定时器回调函数类型
    using Timestamp = int64_t;                      // 时间戳数据类型
    using TimeType = int64_t;                       // 其他时间类型

    const static TimeType kMicroSecondsPerSecond = 1000 * 1000;       //每秒的微秒数 1000 * 1000

    /**
     * 构造函数
     * @param cb 
     * @param timeout
     * @param interval
     */
    Timer(const TimerCallback& cb, Timestamp timeout, TimeType interval);

    /**
     * 执行回调
     */
    void run() const;

    /**
     * 获取定时器的到期时间
     * @return 到期时间戳
     */
    Timestamp getExpiredTime() const {
        return expiredTime_;
    }

    /**
     * 更新到期时间
     * @param newTimeout
     */
    void updateExpiradTime(Timestamp newTimeout) {
        expiredTime_ = newTimeout;
    }

    /**
     * 是否周期性执行
     */
    bool repeat() const {
        return repeat_;
    }

    /**
     * 获取定时器序列号
     * @return 序列号
     */
    int64_t getSequence() const {
        return sequence_;
    }

    /**
     * 判断是否有效时间戳，大于当前时间戳才被认为是有效可用的
     * @return true/false
     */
    bool isValid();

    /**
     * 返回无效时间戳
     */
    Timestamp invalid() const {
        return 0;
    }

    void restart(Timestamp now);

    /**
     * 获取当前时间戳，即自 1970-01-01 00:00:00 的微秒
     * @return 微秒
     */
    static Timestamp now();

    //获取一个序列号
    static int64_t createNum();

private:
    const TimerCallback callback_;  // 定时回调函数
    Timestamp expiredTime_;         // 到期时间戳
    const double interval_;     // 执行周期
    const bool repeat_;         // 是否周期性执行
    const int64_t sequence_;    // 定时器序列号

    static AtomicInt64 s_numCreated_;   //序列号生成器
};
} // namespace net

} // namespace chive


#endif