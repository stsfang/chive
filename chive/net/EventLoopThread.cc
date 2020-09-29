#include "chive/net/EventLoopThread.h"
#include "chive/net/EventLoop.h"
#include "chive/base/clog/chiveLog.h"

using namespace chive;
using namespace chive::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                 const std::string& name)
    : loop_ (NULL),
    exiting_ (false),
    thread_ (std::bind(&EventLoopThread::threadFunc, this), name),
    mutex_ (),
    cond_ (mutex_),
    callback_ (cb)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != NULL)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    assert(!thread_.started());
    thread_.start();    // 创建线程
    EventLoop* loop = NULL;
    {
        MutexLockGuard lock(mutex_);
        while (loop_ == NULL)
        {
            cond_.wait();
        }
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop; // stack obj
    if (callback_)
    {
        callback_(&loop);
    }

    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        CHIVE_LOG_DEBUG("loop2 %p", loop_);
        cond_.notify();
    }

    loop_->loop();
    /// FIXME:是否必要?
    /// loop结束对loop_置空
    // MutexLockGuard lock(mutex_);
    // loop_ = nullptr;
}