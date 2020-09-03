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
class Poller : chive::noncopyable
{
public:
    using ChannelList = std::vector<Channel*> ;
    Poller(EventLoop* evloop);
    ~Poller();

    int poll(int timeoutMs, ChannelList* activeChannels);
    void updateChannel(Channel* channel)  __attribute__ ((optimize(0)));

private:
    using ChannelMap = std::map<int, Channel*>;
    using PollFdList = std::vector<struct pollfd>;

    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
    PollFdList pollfds_;
    ChannelMap channels_;
    EventLoop* ownerLoop_;
};
} // namespace net

} // namespace chive

#endif