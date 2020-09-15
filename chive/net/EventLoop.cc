#include "chive/net/EventLoop.h"
// #include "chive/net/Channel.h"
// #include "chive/net/poller.h"


#include <sys/poll.h>
#include <iostream>
#include <cassert>

using namespace chive;
using namespace chive::net;

// 局部线程存储
__thread EventLoop* t_loopInThisThread = nullptr;
//
const int kPollTimeMs = 10000;
//静态方法
EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

EventLoop::EventLoop():
    looping_(false),
    quit_(false),
    threadId_(CurrentThread::tid()),
    callingPendingFunctors_(false),
    poller_(new Poller(this))
{
    if(t_loopInThisThread)
    {

    }
    else 
    {
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    t_loopInThisThread = nullptr;
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