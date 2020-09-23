#include "chive/net/Channel.h"
#include "chive/net/EventLoop.h"

#include <sys/poll.h>

using namespace chive;
using namespace chive::net;

// === static data member declaration ===
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *evloop, int fd) : loop_(evloop),
                                              fd_(fd),
                                              events_(0),
                                              revents_(0),
                                              index_(-1),
                                              eventHanding_(false),
                                              addedToLoop_(false),
                                              tied_(false)

{
}

Channel::~Channel()
{
    assert(!eventHanding_);
    assert(!addedToLoop_);
    if (loop_->isInLoopThread())
    {
        assert(!loop_->hasChannel(this));
    }
}

void Channel::tie(const std::shared_ptr<void>& obj)
{
    tie_ = obj;
    tied_ = true;
}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::handleEvent()
{
    std::shared_ptr<void> guard;
    if (tied_)
    {
        guard = tie_.lock();
        if (guard)
        {
            handleEventWithGuard(0);
        }
    }
    else 
    {
        handleEventWithGuard(0);
    }
}

void Channel::handleEventWithGuard(Timer::Timestamp receiveTime)
{
    if (revents_ & (POLLERR | POLLNVAL))
    {
        if (errorCallback_)
            errorCallback_();
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if (readCallback_)
            readCallback_();
    }
    if (revents_ & POLLOUT)
    {
        if (writeCallback_)
            writeCallback_();
    }
}


void Channel::disableWriting() 
{
    events_ &= ~kWriteEvent;
    update();
}

void Channel::disableAll()
{
    events_ = kNoneEvent;
    update();
}

bool Channel::isWriting()
{
    return static_cast<bool>(events_ & kWriteEvent);
}

void Channel::remove()
{
    assert(isNoneEvent());
    addedToLoop_ = false;
    loop_->removeChannel(this);
}