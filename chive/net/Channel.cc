#include "chive/net/Channel.h"
#include "chive/net/EventLoop.h"

#include <sys/poll.h>

using namespace chive;
using namespace chive::net;

// === static data member declaration === 
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* evloop, int fd):
    loop_(evloop),
    fd_(fd),
    events_(0),
    revents_(0),
    index_(-1)
{
}

// Channel::~Channel()
void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::handleEvent()
{
    if(revents_ & (POLLERR | POLLNVAL)) {
        if(errorCallback_) errorCallback_();
    }
    if(revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if(readCallback_) readCallback_();
    }
    if(revents_ & POLLOUT) {
        if(writeCallback_) writeCallback_();
    }
}

