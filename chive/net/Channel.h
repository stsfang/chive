#ifndef CHIVE_NET_CHANNEL_H
#define CHIVE_NET_CHANNEL_H
#include "chive/base/noncopyable.h"

#include <functional>

namespace chive
{
namespace net
{
class EventLoop;

class Channel: chive::noncopyable 
{
public:
    using EventCallback = std::function<void()>;
    Channel(EventLoop* evloop, int fd);
    ~Channel() = default;

    void handleEvent();

    // ==== set event callbasks
    void setReadCallback(const EventCallback& cb) {
        readCallback_ = cb;
    }
    void setWriteCallback(const EventCallback& cb) {
        writeCallback_ = cb;
    }
    void setErrorCallback(const EventCallback& cb) {
        errorCallback_ = cb;
    }

    int getFd() { return fd_;}
    int getEvents() { return events_;}
    void setRevents(int revts) { revents_ = revts; }
    bool isNoneEvent() const { return events_ == kNoneEvent; }

    // === enable events ===
    void enableReading() {
        events_ |= kReadEvent;
        update();
    }
    void enableWriting() {
        events_ |= kWriteEvent;
        update();
    }
    // === FIXME: disable events? ===

    // === for poller ===
    int getIndex() {return index_; }
    void setIndex(int idx) { index_ = idx; }

    EventLoop* getOwnerLoop() { return loop_; }

private:
    void update();
    // === event number ====
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
};
} // namespace net

} // namespace chive
#endif