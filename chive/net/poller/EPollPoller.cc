#include "chive/net/poller/EPollPoller.h"
#include "chive/base/clog/chiveLog.h"

#include <cassert>
#include <sys/epoll.h>
#include <error.h>  // errno
#include <string.h> //strerror()
/*
epoll_create(int size)    // linux 独有
epoll_create1(int flags)  // >= kernel version >= 2.6.27, glibc version >= 2.9
如不指定flags, epoll_create1 和 epoll_create 没区别
*/


using namespace chive::net;

namespace
{
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
}

EPollPoller::EPollPoller(EventLoop* loop)
    : Poller(loop),     /*基类poller构造函数*/
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize)
{
    if(epollfd_ < 0)
    {
        CHIVE_LOG_ERROR("create epollfd failed!, epollfd_ %d", epollfd_);
    }
    CHIVE_LOG_DEBUG("created epoller %p with epollfd %d", this, epollfd_);
}

EPollPoller::~EPollPoller()
{
    CHIVE_LOG_DEBUG("epoller closed.")
    ::close(epollfd_);  //关闭内核事件表
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    CHIVE_LOG_DEBUG("epoller %p begins polling...", this);
    CHIVE_LOG_DEBUG("epoller %p now registerd channels total number %d", this, channels_.size());
    // 开始监听epollfd
    int numEvents = ::epoll_wait(epollfd_,
                                 /*&*events_.begin(),*/
                                 events_.data(),
                                 static_cast<int>(events_.size()),
                                 timeoutMs);
    int savedErrno = errno;         //errno是全局共享的，所以需要本地保存
    Timer::Timestamp now = Timer::now();    //当前时间戳
    if (numEvents > 0)
    {
        // 填充活跃的channel
        CHIVE_LOG_INFO("epoller %p get active event number %d", this, numEvents);
        fillActiveChannels(numEvents, activeChannels);

        // 如果注册的event达到了events_的容量，需要给events_扩容
        // 扩容的策略是每次double

        // events_.size() 返回类型是 size_type 
        if (static_cast<EventList::size_type>(numEvents) == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0)
    {
        CHIVE_LOG_DEBUG("epoller %p didn't get any active channel", this);
    }
    else 
    {
        CHIVE_LOG_ERROR("epoller %p get some err happened, erron %s", this, strerror(errno));
        errno = savedErrno;     /// 将本地的errno写回全局errno
                                /// FIXME: 是否增加errno的handler
    }
    return now;
}

void EPollPoller::updateChannel(Channel* channel)
{
    Poller::assertInLoopThread();   // 调用基类方法
    const int index = channel->getIndex();  // channel.index_ 初始 -1
    CHIVE_LOG_DEBUG("fd = %d events = %d status = %d",
                        channel->getFd(), channel->getEvents(), index);

    if(index == kNew || index == kDeleted)
    {
        int fd = channel->getFd();
        if (index == kNew)
        {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else 
        {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel); /// ? channel移除了但没有从map移除
        }
        channel->setIndex(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else 
    {
        int fd = channel->getFd(); (void) fd;
        ///FIXME: only enable in DEBUG mode
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent()) // 
        {
            update(EPOLL_CTL_DEL, channel);
            channel->setIndex(kDeleted);
        }
        else 
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel* channel)
{
    Poller::assertInLoopThread();
    int fd = channel->getFd();
    CHIVE_LOG_DEBUG("poller %p remove channel %p with socket fd = %d", 
                            this, channel, fd);
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());

    int index = channel->getIndex();
    assert(index == kAdded || index == kDeleted);
    size_t n = channels_.erase(fd); 
    assert(n == 1); (void)n;

    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(kNew);
}

/// private

void EPollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    CHIVE_LOG_DEBUG("numEvents %d events_size %d", numEvents, events_.size());
    assert(static_cast<EventList::size_type>(numEvents) <= events_.size());
    for (int i = 0; i < numEvents; ++i)
    {
        // 在EPollPoller::update() 中, 使用 epoll_event.data.ptr保存channel指针
        // 所以这里可以从中取出channel
        auto *channel = static_cast<Channel*>(events_[i].data.ptr);
#ifndef NDEBUG
        int fd = channel->getFd();
        auto it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);
#endif
        channel->setRevents(events_[i].events);     /// 设置channel上到来的事件
        activeChannels->push_back(channel);          /// 填充到activeChannels
    }
}

void EPollPoller::update(int operation, Channel* channel)
{
    epoll_event event{};
    // struct epoll_event {
    //     unint32_t events;
    //     poll_data_t data;
    // };

    // struct union epoll_data {
    //     void* ptr;
    //     int fd;
    //     uint32_t u32;
    //     unit64_t u64;
    // } epoll_data_t;

    event.events = static_cast<uint32_t>(channel->getEvents());
    event.data.ptr = channel;
    int fd = channel->getFd();
    // 向内核事件表epollfd_ 上的fd执行operation操作
    // EPOLL_CTL_ADD /EPOLL_CTL_MOD/ EPOLL_CTL_DEL
    if (epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        CHIVE_LOG_ERROR("epoll_ctl op = %s fd = %d", 
                                operationToString(operation), fd)
    }
}

/// static members
std::string EPollPoller::operationToString(int op)
{
    switch (op)
    {
    case EPOLL_CTL_ADD:
        return "epoll add";    
    case EPOLL_CTL_MOD:
        return "epoll modify";
    case EPOLL_CTL_DEL:
        return "epoll delete";
    default:
        assert(false);  // 非法操作，便于调试阶段暴露
        return "Unknown Operation";
    }
}