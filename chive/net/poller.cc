#include "chive/net/poller.h"
#include "chive/base/Logger.h"

using namespace chive;
using namespace chive::net;


Poller::Poller(EventLoop* loop):ownerLoop_(loop) {}

Poller::~Poller(){}

int Poller::poll(int timeoutMs, ChannelList* activeChannels)
{
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    //FIXME: record timestamp
    if(numEvents > 0) {
        std::cout << "fill events" << std::endl;
        fillActiveChannels(numEvents, activeChannels);
    } else if(numEvents == 0) {
        
    } else {

    }
    numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    //FIXME: should return timestamp
    return 0;
}

void Poller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const 
{
    debug() << "trace in Poller::fillActiveChannels()" << std::endl;
    using poller_iter = PollFdList::const_iterator;
    for(auto pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; ++pfd)
    {
        // 将监听到的fd及其上的events封装到Channel
        // 填充到activeChannels返回给调用者
        if(pfd->revents > 0)
        {
            --numEvents;
            ChannelMap::const_iterator ch = channels_.find(pfd->fd);
            Channel* channel = ch->second;
            channel->setRevents(pfd->revents);
            activeChannels->push_back(channel);
        }
    }
}

void Poller::updateChannel(Channel* channel)
{
    std::cout << "fd = " << channel->getFd() << " events = " << channel->getEvents() << std::endl;
    if(channel->getIndex() < 0) // a new one, add to pollfds_
    {
        struct pollfd pfd;
        pfd.fd = channel->getFd();
        pfd.events = static_cast<short>(channel->getEvents());
        pfd.revents = 0;
        std::cout <<"poll fds size "; 
        std::cout << pollfds_.size() << std::endl;
        pollfds_.push_back(pfd);
        int idx = static_cast<int>(pollfds_.size()) - 1;
        channel->setIndex(idx);
        channels_[pfd.fd] = channel;
    }
    else // update a existing one
    {
        assert(channels_.find(channel->getFd()) != channels_.end());
        assert(channels_[channel->getFd()] == channel);
        int idx = channel->getIndex();
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
        struct pollfd& pfd = pollfds_[idx];
        assert(pfd.fd == channel->getFd() || pfd.fd == -1);
        pfd.events = static_cast<short>(channel->getEvents());
        pfd.revents = 0;
        if(channel->isNoneEvent()) { pfd.fd = -1; }
    }
}