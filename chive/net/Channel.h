#ifndef CHIVE_NET_CHANNEL_H
#define CHIVE_NET_CHANNEL_H
#include "chive/base/noncopyable.h"

#include <functional>
#include <memory>
namespace chive
{
namespace net
{
class EventLoop;
class Channel: noncopyable 
{
public:
    using EventCallback = std::function<void()>;

    Channel(EventLoop* evloop, int fd);
    ~Channel();

    /**
     * channel核心，根据不同的revents值调用对应的回调函数
     */
    void handleEvent();

    /**
     * 设置回调函数
     */
    void setReadCallback(const EventCallback& cb) {
        readCallback_ = cb;
    }
    void setWriteCallback(const EventCallback& cb) {
        writeCallback_ = cb;
    }
    void setErrorCallback(const EventCallback& cb) {
        errorCallback_ = cb;
    }

    /**
     * 绑定对象
     */
    void tie(const std::shared_ptr<void>&);

    int getFd() { return fd_;}
    int getEvents() { return events_;}
    void setRevents(int revts) { revents_ = revts; }
    /**
     * 是否未设置事件
     */
    bool isNoneEvent() const { return events_ == kNoneEvent; }

    /**
     * 开启可读，将fd更新到loop
     */
    void enableReading() {
        events_ |= kReadEvent;
        update();
    }
    /**
     * 开启可写，将fd事件更新到loop
     */
    void enableWriting() {
        events_ |= kWriteEvent;
        update();
    }

    /**
     * 设置不可写，原因
     * Epoll 采用 LT 模式，只需要在需要时关注写事件
     * 否则socket fd一直可写会频繁唤醒IO线程造成busy loop
     */
    void disableWriting();

    /**
     * 设置全部事件不可用
     */
    void disableAll();

    /**
     * 是否正在写数据
     */
    bool isWriting();

    /**
     * 从EventLoop中移除该Channel
     */
    void remove();

    // === for poller ===
    int getIndex() {return index_; }
    void setIndex(int idx) { index_ = idx; }

    /**
     * 获取channel所属的event loop
     */
    EventLoop* getOwnerLoop() { return loop_; }

private:
    using Timestamp = uint64_t;
    /**
     * 通过loop将fd及其事件更新到poller
     */
    void update();

    /**
     * handleEvent的核心
     */
    void handleEventWithGuard(Timestamp receiveTime);
    
    /**
     * 事件编号,需要include POSIX头文件
     */
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    // === data members ===
    EventLoop* loop_;
    const int fd_;
    int events_;
    int revents_;
    int index_; // indicates whether this channel is added to poller
                // if added, index_ >= 0
    
    // === callbacks ===
    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;

    bool eventHanding_;                 /// 是否在处理事件
    bool addedToLoop_;                  /// 是否添加到EventLoop中
    std::weak_ptr<void> tie_;           /// 绑定对象
    bool tied_;                         /// 是否绑定了对象
};
} // namespace net

} // namespace chive
#endif