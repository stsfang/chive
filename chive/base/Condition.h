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
        int flag = pthread_cond_init(&cond_, nullptr);
        assert(flag == 0); (void)flag;
    }

    ~Condition() {
        int flag = pthread_cond_destroy(&cond_);
        assert(flag == 0); (void)flag;
    }

    void wait() {
        /// FIXME: need lock guard or not?
        int flag = pthread_cond_wait(&cond_, mutex_.getPthreadMutexPtr());
        assert(flag == 0); (void)flag;
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
        int flag = pthread_cond_signal(&cond_);
        assert(flag == 0); (void)flag;
    }

    void notifyall() {
        int flag = pthread_cond_broadcast(&cond_);
        assert(flag == 0); (void)flag;
    }

private:
    MutexLock& mutex_;
    pthread_cond_t cond_;
};
} // namespace chive

#endif