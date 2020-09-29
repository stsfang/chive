#ifndef CHIVE_BASE_MUTEXLOCK_H
#define CHIVE_BASE_MUTEXLOCK_H

#include "chive/base/noncopyable.h"
#include "chive/base/CurrentThread.h"

#include <pthread.h>
#include <stdio.h>
#include <assert.h>


namespace chive
{
/**
 * MutexLock封装pthread mutex
 */
class MutexLock : noncopyable {
public:
    MutexLock() : mutex_{}, holder_(0) {
        int flag = pthread_mutex_init(&mutex_, nullptr);
        assert(flag == 0); (void)flag;
    }

    ~MutexLock() {
        assert(holder_ == 0);
        int flag = pthread_mutex_destroy(&mutex_);
        assert(flag == 0); (void)flag;
    }

    /**
     * 当前线程是否为加锁线程
     * @return
     */
    bool isLockedByThisThread() {
        return holder_ == CurrentThread::tid();
    }

    void assertLocked() {
        bool flag = isLockedByThisThread();
        assert(flag); (void)flag;
    }

    void lock() {
        int flag = pthread_mutex_lock(&mutex_);
        assert(flag == 0); (void)flag;
        holder_ = CurrentThread::tid();
    }

    void unlock() {
        holder_ = 0;
        int flag = pthread_mutex_unlock(&mutex_);
        assert(flag == 0); (void)flag;
    }

    pthread_mutex_t* getPthreadMutexPtr() {
        return &mutex_;
    }

private:
    pthread_mutex_t mutex_;
    pid_t holder_;          //持有该锁的线程Id
};

/**
 * MutexLockGuard：RAII管理加锁和解锁
 * 创建时构造-加锁，离开作用域时，析构-解锁
 */
class MutexLockGuard: noncopyable {
public:
    explicit MutexLockGuard(MutexLock &mutex):mutex_(mutex) {
        mutex_.lock();
    }
    ~MutexLockGuard() {
        mutex_.unlock();
    }
private:
    MutexLock& mutex_;  //声明为引用类型
};

// 防止类似误用 MutexLockGuard(mutex_)
// 临时对象不能长时间持有锁，产生后又马上销毁
// 正确写法 MutexLockGuard lock(mutex_)
#define MutexLockGuard(x) error "Missing guard object name"
} // namespace chive

#endif