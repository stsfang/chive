#include "chive/net/Timer.h"

using namespace chive;
using namespace chive::net;

std::atomic<int64_t> s_numCreated_ {0};

void Timer::restart(Timestamp now)
{
    if(repeat_)
    {
        expirationo_ = addTime(now, interval_);
    }
    else
    {
        expiration_ = Timestamp::invalidInstance();
    }
}