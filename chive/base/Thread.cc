#include "chive/base/Thread.h"
#include "chive/base/clog/chiveLog.h"

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h> 

using namespace chive;
// using namespace chive::base;

AtomicInt32 Thread::numCreated_;

Thread::Thread(const ThreadFunc& func, const std::string& name)
    : started_ (false),
      joined_ (false),
      pthreadId_ (0),
      tid_ (0),
      func_ (std::move(func)),
      name_ (name),
      latch_(1)
{
    setDefaultName();
}

Thread::~Thread()
{
    if (started_ && !joined_) 
    {   /// detach 线程，由系统接管
        pthread_detach(pthreadId_);
    }
}

void Thread::setDefaultName()
{
    int num = numCreated_.incrementAndGet();
    if (name_.empty())
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "Thread#%d", num);
        name_ = buf;
    }
}

void Thread::start()
{
    assert(!started_);
    started_ = true;

    if (0 != pthread_create(&pthreadId_, NULL, &startThread, this))
    {
        started_ = false;
        CHIVE_LOG_ERROR("pthread_create failed!");
    }
    else
    {
        CHIVE_LOG_DEBUG("create thread %p with %ld", this, pthreadId_);
        latch_.wait();
        assert(tid_ > 0);
    }
}

int Thread::join()
{
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL);
}

pid_t Thread::gettid()
{
    return static_cast<pid_t>(syscall(__NR_gettid));
}


void* Thread::startThread(void* obj)
{
    auto* thread = static_cast<Thread*>(obj);
    thread->runInThread();
    return nullptr;
}

void Thread::runInThread()
{
    tid_ = Thread::gettid();
    CHIVE_LOG_DEBUG("latch down...");
    try
    {
        func_();    // 执行注册给线程的函数
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        /// FIXME: 增加crash堆栈
        // fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
        abort();
    }
    catch (...)
    {
        fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
        throw;    //rethrow
    }
}