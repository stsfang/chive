#ifndef CHIVE_NET_TIMERID_H
#define CHIVE_NET_TIMERID_H

#include "chive/base/noncopyable.h"
#include "chive/net/Timer.h"


namespace chive
{
namespace net
{

// class Timer;

// 提供给用户用于标识定时器
// 用于注销定时器队列中的定时器
// Timer是非线程安全的所以不能直接给用户
class TimerId 
{
public:
        TimerId() : sequence_(0) {}
        explicit TimerId(const std::weak_ptr<Timer> &timer)
            : timer_(timer),
              sequence_(timer.lock()? timer.lock()->getSequence() : 0)
        {}

        friend class TimerQueue;

private:
    std::weak_ptr<Timer> timer_;    //使用弱指针
    int64_t sequence_;

};

} // namespace net
} // namespace chive   

#endif