#ifndef CHIVE_BASE_THREAD_H
#define CHIVE_BAST_THREAD_H

#include "chive/base/noncopyable.h"
#include "chive/base/Atomic.h"
#include "chive/base/CountDownLatch.h"

#include <string>
#include <pthread.h>
#include <string>
#include <unistd.h>
#include <functional>

namespace chive
{
// namespace base
// {
class Thread : noncopyable
{
public:
    using ThreadFunc = std::function<void ()>;
    
    explicit Thread(const ThreadFunc&, const std::string& name = std::string());

    ~Thread();

    void start();

    int join();

    bool started() const { return started_; }

    pid_t tid() const { return tid_; }

    const std::string& name() const { return name_; }

    static int numCreated() { return numCreated_.get(); }

    static pid_t gettid();

    void CountDown()
    {
        latch_.countDown();
    }
private:
    void setDefaultName();

    bool started_;                  /// 
    bool joined_;                   /// 
    pthread_t pthreadId_;           /// POSIX tid
    pid_t tid_;                     /// Kernel tid
    ThreadFunc func_;               /// 线程函数
    std::string name_;              /// 线程名
    CountDownLatch latch_;

    static AtomicInt32 numCreated_;


    /**
     * 线程创建时传入的函数指针
     * @param obj 传入线程对象
     */
    static void* startThread(void* obj);

    /**
     * 执行注册给线程的函数
     */
    void runInThread();
};
// } // namespace base


} // namespace chive

#endif