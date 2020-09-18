#include "chive/net/poller/EPollPoller.h"
#include "chive/base/Logger.h"

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
        debug(LogLevel::ERROR) << "EPollPoller::EPollPoller() create epollfd failed";
    }
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);  //关闭内核事件表
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    debug() << "fd total count " << channels_.size();
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
        debug() << numEvents << " events happend";
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
        debug() << "EPollPoller::poll() didn't get any active channel";
    }
    else 
    {
        debug() << "EPollPoller::poll() some err happened, erron " << strerror(errno);
        errno = savedErrno;     /// 将本地的errno写回全局errno
                                /// FIXME: 是否增加errno的handler
    }
    return now;
}

void EPollPoller::updateChannel(Channel* channel)
{
    Poller::assertInLoopThread();   // 调用基类方法
    const int index = channel->getIndex();  // channel.index_ 初始 -1
    debug() << "fd = " << channel->getFd()
            << " events = " << channel->getEvents()
            << " index = " << index;

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
            assert(channels_.find(fd) != channels_end());
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
    debug() << "fd = " << fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());

    int index = channel->getIndex();
    assert(index == kAdded || index == kDeleted);
    size_t n = channels_.erase(fd); (void)n;
    assert(n == 1);

    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(kNew);
}

/// private

void EPollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    assert(static_cast<EventList::size_type>(numEvents) == events_.size());
    for (int i = 0; i < numEvents; ++i)
    {
        // 在EPollPoller::update() 中, 使用 epoll_event.data.ptr保存channel指针
        // 所以这里可以从中取出channel
        auto *channel = static_cast<Channel*>(events_[i].data.ptr);
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
        debug(LogLevel::ERROR) << "epoll_ctl op= " << operationToString(operation)
                                << " fd= " << fd;
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