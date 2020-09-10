#ifndef CHIVE_NET_EVENTLOOP_H
#define CHIVE_NET_EVENTLOOP_H
#include "chive/base/noncopyable.h"
#include <vector>
#include <memory>
#include <atomic>
#include <functional>

//using pid_t = long int;

namespace chive
{
namespace net
{
class Channel;
class Poller;

class EventLoop : chive::noncopyable
{
public:
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();

    // must be called by the thread creates the object
    void loop();

    inline void assertInLoopThread() {
        if(!isInLoopThread()) {
            abortNotInLoopThread();
        }        
    }
    inline bool isInLoopThread() {
        return threadId_ == std::this_thread::getid();
    }

   void updateChannel(Channel* channel);
   void quit();
private:
    //void abortNotInLoopThread();
    bool looping_;
    bool quit_;
    using ChannelList = std::vector<Channel*>;
    ChannelList activeChannels_;
    std::unique_ptr<Poller> poller_;
};

} // namespace net
} // namespace chive

#endif