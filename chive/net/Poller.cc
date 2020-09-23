#include "chive/net/Poller.h"
#include "chive/base/Logger.h"

using namespace chive;
using namespace chive::net;


Poller::Poller(EventLoop* loop)
    : ownerLoop_(loop) 
{
}

Poller::~Poller() = default;

bool Poller::hasChannel(Channel* channel) const
{
    assertInLoopThread();
    auto it = channels_.find(channel->getFd());
    return it != channels_.end() && it->second == channel;
}
