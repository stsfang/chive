#include "chive/net/TcpConnection.h"
#include "chive/net/EventLoop.h"
#include "chive/net/InetAddress.h"
#include "chive/net/TcpServer.h"
#include "chive/base/clog/chiveLog.h"


using namespace chive;
using namespace chive::net;


void onConnection(const TcpConnectionPtr& conn)
{
    if (conn->isConnected())
    {
        CHIVE_LOG_INFO("new connection %s from %s",
                            conn->name().c_str(),
                            conn->peerAddress().toIpPort().c_str());
    }
    else 
    {
        CHIVE_LOG_DEBUG("connection %s is down", conn->name().c_str());
    }
}

void onMessage(const TcpConnectionPtr& conn, const char* data, ssize_t len)
{
    CHIVE_LOG_DEBUG("receive %d bytes  from connection %s",
                            len,  conn->name().c_str());
}

int main()
{
    startLogPrint(NULL);
    InetAddress listenAddr(9909);
    EventLoop loop;

    TcpServer server(&loop, listenAddr, "chive_tcpserver");
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();

    loop.loop();
}