#ifndef CHIVE_NET_TIMEID_H
#define CHIVE_NET_TIMEID_H

#include <memory>

#include "chive/base/copyable.h"

namespace chive
{
namespace net
{
class Timer;

class TimerId : public chive::copyable
{
public:
    TimerId():
        timer_(nullptr),
        squence_(0)
    {
    }

    TimerId(std::shared_ptr<Timer> timer, int64_t seq):
        timer_(std::move(timer)),
        sequence_(seq)
    { 
    }

    /// declare TimerQueue as friend class
    friend class TimeQueue;

private:
    std::shared_ptr<Timer> timer_;
    int64_t squence_;
};
} // namespace net
} // namespace chive

#endif