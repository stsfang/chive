#ifndef CHIVE_NET_EVENTLOOPTHREADPOOL_H
#define CHIVE_NET_EVENTLOOPTHREADPOLL_H

#include "chive/base/noncopyable.h"
#include <functional>
#include <string>
#include <vector>
#include <memory>

namespace chive
{
namespace net
{
class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* baseLoop, const std::string& name);
    ~EventLoopThreadPool();
    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    /**
     * valid after calling start()
     * round-robin selection
     */
    EventLoop* getNextLoop();

    EventLoop* getLoopForHash(size_t hashCode);

    std::vector<EventLoop*> getAllLoops();

    bool started() const
    { return started_; }

    const std::string& name() const 
    { return name_; }

private:
    EventLoop* baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;

};
} // namespace net

} // namespace chive

#endif