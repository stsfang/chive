#include "chive/net/EventLoop.h"
// #include "chive/net/Channel.h"
// #include "chive/net/poller.h"


#include <sys/poll.h>
#include <iostream>
#include <cassert>

using namespace chive;
using namespace chive::net;

const int kPollTimeMs = 10000;

EventLoop::EventLoop():
    looping_(false),
    quit_(false),
    threadId_(std::this_thread::get_id()),
    callingPendingFunctors_(false),
    poller_(new Poller(this))
{
    
}

EventLoop::~EventLoop()
{
    //assert(!mLooping);
}

void EventLoop::loop()
{
    assert(!looping_);
    looping_ = true;
    quit_ = false;
    while(!quit_)
    {
        activeChannels_.clear();
        poller_->poll(kPollTimeMs, &activeChannels_);
        for(ChannelList::iterator it = activeChannels_.begin();
            it != activeChannels_.end(); it++)
        {
            (*it)->handleEvent();
        }
    }
    std::cout << "loop end" << std::endl;
    looping_ = false;
}

void EventLoop::updateChannel(Channel* channel)
{
    assert(channel->getOwnerLoop() == this);
    poller_->updateChannel(channel);
}

void EventLoop::quit()
{
    quit_ = true;
}