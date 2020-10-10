cc_library(
    name = 'chive_net',
    srcs = [
        'EventLoop.cc',
        'Channel.cc',
        'Poller.cc',
        'poller/EPollPoller.cc',
        'TimerQueue.cc',
        'Timer.cc',
        'Acceptor.cc',
        'Socket.cc',
        'InetAddress.cc',
        'TcpConnection.cc',
        'TcpServer.cc',
        'Buffer.cc',
        'EventLoopThread.cc',
        'EventLoopThreadPool.cc',
        'SocketOps.cc',
        'Connector.cc',
        'TcpClient.cc',
    ],
    deps = [
   #     '//chive/net/poller/:chive_net_poller'
    ],
    extra_cppflags = [
        '-std=c++17',
        # '-Wall',  # show warning as error
        '-w',       # no warning
         '-g',
    ]
)
