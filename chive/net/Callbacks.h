#ifndef CHIVE_NET_CALLBACKS_H
#define CHIVE_NET_CALLBACKS_H

#include <functional>
#include <memory>

namespace chive
{
namespace net
{

class TcpConnection;
class Buffer;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void (const TcpConnectionPtr&)>;
using Timestamp = uint64_t;

using MessageCallback = std::function<void (const TcpConnectionPtr&, Buffer* buf, Timestamp now)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
// 低水位回调
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
// 高水位回调
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, size_t)>;

void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn,
                            Buffer* buffer,
                            Timestamp receiveTime);
} // namespace net

} // namespace chive

#endif