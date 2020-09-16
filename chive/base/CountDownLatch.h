#ifndef CHIVE_BASE_COUNTDOWNLATCH_H
#define CHIVE_BASE_COUNTDOWNLATCH_H

#include "chive/base/noncopyable.h"
#include "chive/base/Condition.h"
#include "chive/base/MutexLock.h"

namespace chive
{
class CountDownLatch : noncopyable {
public:

    explicit CountDownLatch(int count);
    
    void wait();
    
    void countDown();
    
    int getCount() const;
private:
    // 
    mutable MutexLock mutex_;
    // 相比muduo源码，这里省去了clang线程安全检查注解 guard_by
    /// FIXME: 
    Condition condition_;
    int count_;
};
} // namespace chive

#endif