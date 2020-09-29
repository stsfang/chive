#include "chive/net/Channel.h"
#include "chive/net/EventLoop.h"
#include "chive/base/clog/chiveLog.h"

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
    CHIVE_LOG_DEBUG("created channel %p with fd %d in eventloop %p", this, fd, evloop);
}

Channel::~Channel()
{
    assert(!eventHanding_);         // 正在处理事件 不能析构
    assert(!addedToLoop_);          // 没被添加到loop 不能析构
    CHIVE_LOG_DEBUG("channel %p with socket fd %d", this, fd_);
    if (loop_->isInLoopThread())
    {
        assert(!loop_->hasChannel(this));
    }
    CHIVE_LOG_DEBUG("destroyed channel %p with socket fd %d", this, fd_);
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

void Channel::handleEvent(Timestamp receiveTime)
{
    CHIVE_LOG_DEBUG("channel %p handle events", this);
    eventHanding_ = true;           //标志置位 正在处理事件
    std::shared_ptr<void> guard;
    if (tied_)
    {
        guard = tie_.lock();
        if (guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else 
    {
        handleEventWithGuard(receiveTime);
    }
    eventHanding_ = false;      //标志复位 退出处理事件
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
    {
        CHIVE_LOG_WARN("POLLHUP event");
        if (closeCallback_) 
            closeCallback_();
    }
    if (revents_ & (POLLERR | POLLNVAL))
    {
        if (errorCallback_)
            errorCallback_();
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if (readCallback_)
            readCallback_(receiveTime);
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