#ifndef CHIVE_NET_TIMER_H
#define CHIVE_NET_TIMER_H

#include <atomic>

#include "chive/base/noncopyable.h"
#include "chive/base/Timestamp.h"
#include "chive/net/Callbacks.h"

namespace chive : noncopyable
{
namespace net
{
class Timer
{
public:
    Timer(TimerCallback cb, Timestamp when, double interval)
        : callback_(std::move(cb)),
        expiration_(when),
        interval_(interval),
        repeat_(interval > 0.0),
        sequence_(s_numCreated_ ++)
        {
            ///
            ///may should init it by this way
            ///
            //std::atomic_init(s_numCreated_, 0);
            //squence_ = s_numCreated_ ++;
        }

        void run() const { callback_(); }
        Timestamp expiration() const { return expiration_; }
        bool repeat() const { return repeat_; }
        int64_t sequence() const { return sequence_; }
        
        void restart(Timestamp now);

        static int64_t numCreated() { return s_numCreated_.load(); }
 private:
    const TimerCallback callback_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;
    //use c++11 atomic ops 
    static std::atomic<int64_t> s_numCreated;
};
} // namespace net

} // namespace chive

#endif