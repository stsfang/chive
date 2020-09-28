#ifndef CHIVE_EVENTLOOP_THREAD_H
#define CHIVE_EVENTLOOP_THREAD_H

#include "chive/base/noncopyable.h"
#include "chive/base/Thread.h"  // include <functional> <string>
#include "chive/base/Atomic.h"
// #include <functional>

namespace chive
{
namespace net
{

class EventLoop;

class EventLoopThread : noncopyable 
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                    const std::string& name = std::string());
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void threadFunc();

    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    bool canGo = false;
    MutexLock mutex_;
    Condition cond_;
    ThreadInitCallback callback_;
};
} // namespace net

} // namespace chive


#endif