#ifndef CHIVE_NET_POLLER_EPOLLPOLLER_H
#define CHICE_NET_POLLER_EPOLLPOLLER_H

#include "chive/net/Poller.h"   // include "EventLoop.h"
#include <vector>
#include <string>

// 前置声明，在源文件引入
struct epoll_event;

namespace chive
{
namespace net
{

/// IO多路复用 (epoll)
class EPollPoller : public Poller 
{
public:
    EPollPoller(EventLoop* loop);
    ~EPollPoller() override;    //重载

    /**
     * epoll开始监听
     * @param timeoutMs 监听的超时时间
     * @param activeChannels 获取激活的channels
     * @return poll监听返回的时间戳
     */
    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;

    /**
     * 更新channel上的信息到epoll
     * @param channel 待更新的channel
     */
    void updateChannel(Channel* channel) override;

    /**
     * 从epoll上移除channel
     * @param channel 待移除的channel
     */
    void removeChannel(Channel* channel) override;

private:
    using EventList = std::vector<struct epoll_event>;

    /**
     * 填充活跃的channel
     * @param numEvents active状态的事件数
     * @param activeChannels 获取active的channel
     */
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

    /**
     * 在Channel的fd上执行operation操作 (移除|更新|添加 fd事件等)
     * @param operation 操作类型
     * @param channel 被操作的对象
     */
    void update(int operation, Channel* channel);

    int epollfd_;           // 内核事件表 fd
    EventList events_;      // 内核事件表上的事件列表

    static const int kInitEventListSize = 16;       // 

    /**
     * 将操作op操作名转换为字符串
     */
    static std::string operationToString(int op);
};
} // namespace net

} // namespace chive

#endif