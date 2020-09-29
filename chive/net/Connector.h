#ifndef CHIVE_NET_CONNECTOR_H
#define CHIVE_NET_CONNECTOR_H

#include "chive/base/noncopyable.h"
#include <functional>

namespace chie
{
namespace net
{
class EventLoop;
class InetAddress;

class Connector : noncopyable
{
public:
    using NewConnectionCallback = std::function<void (int sockfd)>;

    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { newConnectionCallback_ = cb; }
    
    void start();
    void restart();
    void stop();
private:
    NewConnectionCallback newConnectionCallback_;

};
} // namespace net

} // namespace chie

#endif