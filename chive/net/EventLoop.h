#ifndef CHIVE_NET_EVENTLOOP_H
#define CHIVE_NET_EVENTLOOP_H
#include "chive/base/noncopyable.h"
#include <vector>
#include <memory>
#include <atomic>
#include <functional>
#include <mutex>
#include <thread>

#include "chive/net/TimerId.h"
#include "chive/net/Timer.h"
#include "chive/net/Channel.h"
#include "chive/net/TimerQueue.h"
#include "chive/net/poller.h"

namespace chive
{
namespace net
{

//前置声明
// class Channel;
// class Poller;
// class TimerQueue;
// class TimerId;

class EventLoop : chive::noncopyable
{
public:

    using Functor = std::function<void()>;  //pending task 函数对象类型

    EventLoop();
    ~EventLoop();

    void loop();
    /*
    inline void assertInLoopThread() {
        if(!isInLoopThread()) {
            abortNotInLoopThread();
        }        
    }
    inline bool isInLoopThread() {
        return true;
    }
    */
   void updateChannel(Channel* channel);
   void quit();

    /**
     * 在IO线程里执行用户任务回调，用于线程间调配任务
     * @param cb 待执行的回调函数
     */ 
    void runInLoop(const Functor& cb);

    /**
     * 将任务回调放入到pending队列
     */
    void queueInLoop(const Functor& cb);

    // methods for add Timer to TimerQueue --- begin
    TimerId runAt(Timer::Timestamp time, const Timer::TimerCallback& cb);

    TimerId runAfter(Timer::TimeType delay, const Timer::TimerCallback& cb);

    TimerId runEvery(Timer::TimeType interval, const Timer::TimerCallback& cb);
    // methods for add Timer to TimerQueue --- end

private:
    using ChannelList = std::vector<Channel*>;

    
    void abortNotInLoopThread();

    // add for transferring operations of TimerQueue to IO thread ---- begin
    void handleRead();                           //唤醒IO线程loop，处理pending任务
    void doPendingFunctors();                   //处理pending task
    bool callingPendingFunctors_;               //标识是否正在处理pending task
    int wakeupFd_;                              // wakeup fd 唤醒线程处理pending task
    std::unique_ptr<Channel> wakeupChannel_;    //专用于wakeup fd 上的readable事件，分发给handleRead()
    std::vector<Functor> pendingFunctors_;      //等待处理的回调任务,需要枷锁保护，因为TimerQueue可以在另一个线程访问之
    std::mutex mutex_;                          //互斥量
    // add for transferring operations of TimerQueue to IO thread ---- end


    bool looping_;
    bool quit_;
    std::thread::id threadId_;                  // eventloop所在线程ID
    
    ChannelList activeChannels_;
    std::unique_ptr<Poller> poller_;
};

} // namespace net
} // namespace chive

#endif