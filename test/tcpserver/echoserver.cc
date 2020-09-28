#include "chive/net/TcpServer.h"
#include "chive/net/EventLoop.h"
#include "chive/net/InetAddress.h"
#include "chive/base/clog/chiveLog.h"
#include <stdio.h>

using namespace chive;
using namespace chive::net;

static std::string msg1 = std::string("huhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh")
                        + std::string("llllllllllllllllllllllllllllllllllllllllllllllllllll")
                        + std::string("huuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu");

static std::string msg2 = "hello world";

void onConnection(const TcpConnectionPtr& conn) {
    if (conn->isConnected()) {
        CHIVE_LOG_DEBUG("new connection [%s] from %s",
                            conn->name().c_str(),
                            conn->peerAddress().toIpPort().c_str());
        CHIVE_LOG_DEBUG("sleep....");
        ::sleep(5);

        // conn->send(msg1);
        conn->send(msg2);
        conn->shutdown();
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

void onWriteComplege(const TcpConnectionPtr& conn) {
    conn->send("all sent, bye!");
}

int main() {
    startLogPrint(NULL);

    CHIVE_LOG_DEBUG("main(): pid = %d", getpid());
    InetAddress listenAddr(9909);
    EventLoop loop;

    TcpServer server(&loop, listenAddr, "chive_echoserver");
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.setWriteCompleteCallback(onWriteComplege);
    server.start();

    loop.loop();
}