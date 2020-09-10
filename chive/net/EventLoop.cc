#include <sys/poll.h>
#include <iostream>
#include <cassert>
#include <sys/eventfd.h>
#include <unistd.h>

#include "chive/net/EventLoop.h"
#include "chive/net/Channel.h"
#include "chive/net/poller.h"



using namespace chive;
using namespace chive::net;

///
/// thread local variavle, to limit each thread having no more than one
/// EventLoop instance
///
__thread EventLoop* t_loopInThisThread = nullptr;
const int kPollTimeMs = 10000;

int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    IF(evtfd < 0) {
        
    }
}
EventLoop::EventLoop():
    looping_(false),
    quit_(false),
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