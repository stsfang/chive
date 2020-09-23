#ifndef CHIVE_BASE_CONDITION_H
#define CHIVE_BASE_CONDITION_H

#include "chive/base/MutexLock.h"

#include <pthread.h>
#include <cassert>
#include <errno.h>  /*provide ETIMEDOUT */

namespace chive
{
class Condition : noncopyable {
public:
    explicit Condition(MutexLock& mutex) : mutex_(mutex) {
        assert(pthread_cond_init(&cond_, nullptr) == 0);
    }

    ~Condition() {
        assert(pthread_cond_destroy(&cond_) == 0);
    }

    void wait() {
        /// FIXME: need lock guard or not?
        assert(pthread_cond_wait(&cond_, mutex_.getPthreadMutexPtr()) == 0);
    }

    bool waitForSecond(int second) {
        struct timespec timeout{};
        // CLOCK_REALTIME 和 CLOCK_MONOTONIC 的区别
        // clock_getres 和 clock_gettime的区别
        /*clock_getres(CLOCK_REALTIME, &timeout);*/
        clock_getres(CLOCK_MONOTONIC, &timeout);
        timeout.tv_sec += second;
        return pthread_cond_timedwait(
            &cond_, mutex_.getPthreadMutexPtr(), &timeout) == ETIMEDOUT;
    }

    void notify() {
        assert(pthread_cond_signal(&cond_) == 0);
    }

    void notifyall() {
        assert(pthread_cond_broadcast(&cond_) == 0);
    }

private:
    MutexLock& mutex_;
    pthread_cond_t cond_;
};
} // namespace chive

#endif