#ifndef CHIVE_NET_CALLBACKS_H
#define CHIVE_NET_CALLBACKS_H

#include <functional>
#include <memory>

namespace chive
{
namespace net
{

class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void (const TcpConnectionPtr&)>;
using Timestamp = unit64_t;
/*
using MessageCallback 
            = std::function<void (const TcpConnectionPtr&, void*, Timestamp)>;
*/
using MessageCallback = std::function<void (const TcpConnectionPtr&, char* buf, int len)>;


using 
} // namespace net

} // namespace chive

#endif