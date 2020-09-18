#ifndef CHIVE_NET_POLLER_H
#define CHIVE_NET_POLLER_H

#include "chive/base/noncopyable.h"
#include "chive/net/Channel.h"
#include "chive/net/EventLoop.h"

#include <vector>
#include <map>
#include <sys/poll.h>
#include <iostream>
#include <cassert>

namespace chive
{
namespace net
{
using Timestamp = int64_t;
class Poller : chive::noncopyable
{
public:
    using ChannelList = std::vector<Channel*> ;
    Poller(EventLoop* evloop);
    virtual ~Poller();

    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0;
    virtual bool hasChannel(Channel* channel) const;

    static Poller* newDefaultPoller(EventLoop* loop);

    void assertInLoopThread() const 
    {
        ownerLoop_->assertInLoopThread();
    }

protected:
    using ChannelMap = std::map<int, Channel*>;
    // remove PollFdList/fillActiveChannels
    // using PollFdList = std::vector<struct pollfd>;
    // void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
    // PollFdList pollfds_;
    ChannelMap channels_;
    EventLoop* ownerLoop_;
};
} // namespace net

} // namespace chive

#endif