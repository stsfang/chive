#include "chive/net/EventLoopThreadPool.h"
#include "chive/net/EventLoop.h"
#include "chive/net/EventLoopThread.h"
#include "chive/base/clog/chiveLog.h"

#include <cassert>
#include <stdio.h>

using namespace chive;
using namespace chive::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string& name)
    : baseLoop_ (baseLoop),
      name_ (name),
      started_ (false),
      numThreads_ (0),
      next_ (0)
{
    CHIVE_LOG_DEBUG("created eventloopthreadpool %p name %s", this, name_.c_str());
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    // all loops are stack obj
    CHIVE_LOG_DEBUG("eventloopthreadpool %p name %s destructed.", this, name_.c_str());
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
    assert(!started_);
    baseLoop_->assertInLoopThread();
    started_ = true;

    char buf[name_.size() + 32];
    for (int i = 0; i < numThreads_; ++i)
    {
        memset(buf, '\0', sizeof(buf));
        snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
        EventLoopThread* t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        // 得到loop*
        loops_.push_back(t->startLoop());
    }
    // 如果是单线程服务就返回baseLoop_
    if (numThreads_ == 0 && cb)
    {
        cb(baseLoop_);
    }
}

// round-robin
EventLoop* EventLoopThreadPool::getNextLoop()
{
    baseLoop_->assertInLoopThread();
    assert(started_);
    EventLoop* loop = baseLoop_;

    if (!loops_.empty())
    {
        loop = loops_[next_];
        ++next_;
        if (static_cast<size_t>(next_) >= loops_.size())
        {
            next_ = 0;
        }
    }
    return loop;
}

// hash code
EventLoop* EventLoopThreadPool::getLoopForHash(size_t hashCode)
{
    baseLoop_->assertInLoopThread();
    EventLoop* loop = baseLoop_;

    if (!loops_.empty())
    {
        loop = loops_[hashCode % loops_.size()];
    }
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
    baseLoop_->assertInLoopThread();
    assert(started_);
    if (loops_.empty())
    {
        return std::vector<EventLoop*>(1, baseLoop_);
    }
    else 
    {
        return loops_;
    }
}