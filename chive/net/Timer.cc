#include "chive/net/Timer.h"

#include <sys/time.h>
#include <memory>

using namespace chive;
using namespace chive::net;

//静态变量初始化
AtomicInt64 Timer::s_numCreated_;

Timer::Timer(const TimerCallback& cb, Timestamp timeout, TimeType interval) 
    : callback_(cb),
      expiredTime_(timeout),
      interval_(interval),
      repeat_(interval_ > 0),
      sequence_(s_numCreated_.incrementAndGet()) {

}

void Timer::run() const {
    callback_();
}

bool Timer::isValid() {
    return expiredTime_ >= Timer::now();
}

void Timer::restart(Timestamp now) {
    if(repeat_) {
        //周期执行，给当前到期时间续命一个周期
        expiredTime_ = now + interval_;
    } else {
        expiredTime_ = invalid();
    }
}

/**
 * 获取当前系统时间戳（微秒单位）
 */
Timer::Timestamp Timer::now() {
    /**
     * timeval {
     * time_t tv_sec;
     * suseconds_t  tv_usec;
     * }
     */
    std::shared_ptr<timeval> tv(std::make_shared<timeval>());
    // 第二个参数用于指定timezone
    gettimeofday(tv.get(), nullptr);    //该函数不是系统调用，deprecated

    //获取秒 * 1000*1000 + 微秒 得到基于微秒的时间戳
    return static_cast<int64_t>(tv->tv_sec * Timer::kMicroSecondsPerSecond 
                                    + tv->tv_usec);
}

int64_t Timer::createNum() {
    return s_numCreated_.get();
}