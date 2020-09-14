#ifndef CHIVE_BASE_ATOMIC_H
#define CHIVE_BASE_ATOMIC_H

#include "chive/base/noncopyable.h"
#include <cstdint>

namespace chive
{
namespace base
{
template<typename T>
class AtomicIntegerT : noncopyable
{
public:
    AtomicIntegerT():value_(0){}
    
    T get() {
        return __atomic_load_n(&value_, __ATOMIC_SEQ_CST);
    }

    T getAndAdd(T x) {
        return __atomic_fetch_add(&value_, x, __ATOMIC_SEQ_CST);
    }

    T addAndGet(T x) {
        return getAndAdd(x) + x;
    }

    T incrementAndGet() {
        return addAndGet(1);
    }

    T decrementAndGet() {
        return addAndGet(-1);
    }

    void add(T x) {
        getAndAdd(x);
    }

    void increment() {
        incrementAndGet();
    }

    void decrement() {
        decrementAndGet();
    }

    T getAndSet(T newVal) {
        return __atomic_exchange_n(&value_, newVal, __ATOMIC_SEQ_CST);
    }

private:
    volatile T value_;
};
} // namespace base

/**
 * 提供给chive::net namespace下直接使用
 */
using AtomicInt32 = base::AtomicIntegerT<int32_t>;
using AtomicInt64 = base::AtomicIntegerT<int64_t>;
} // namespace chive

#endif