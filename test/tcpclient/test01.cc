#include "chive/base/clog/chiveLog.h"
#include "chive/net/EventLoopThread.h"
#include "chive/net/TcpClient.h"

#include <unistd.h>

using namespace chive;
using namespace chive::net;

int main(int argc, char* argv[])
{
    startLogPrint(nullptr);

    EventLoopThread loopThread;
    {
        InetAddress serverAddress("127.0.0.1", 1234);
        TcpClient client(loopThread.startLoop(), serverAddress, "TcpClient");
        client.connect();
        sleep(50);
        client.disconnect();
    }

    sleep(100);
}