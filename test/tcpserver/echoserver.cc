#include "chive/net/TcpServer.h"
#include "chive/net/EventLoop.h"
#include "chive/net/InetAddress.h"
#include "chive/base/clog/chiveLog.h"
#include <stdio.h>

using namespace chive;
using namespace chive::net;

void onConnection(const TcpConnectionPtr& conn) {
    if (conn->isConnected()) {
        CHIVE_LOG_DEBUG("new connection [%s] from %s",
                            conn->name().c_str(),
                            conn->peerAddress().toIpPort().c_str());
    } else {
        CHIVE_LOG_WARN("connecion [%s] is down", conn->name().c_str());
    }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime) {
    CHIVE_LOG_DEBUG("receive %d bytes from connection [%s] at %ld",
                            buf->readableBytes(),
                            conn->name().c_str(),
                            receiveTime);
    conn->send(buf->retrieveAllAsString());
}

int main() {
    startLogPrint(NULL);

    CHIVE_LOG_DEBUG("main(): pid = %d", getpid());
    InetAddress listenAddr(9909);
    EventLoop loop;

    TcpServer server(&loop, listenAddr, "chive_echoserver");
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();

    loop.loop();
}