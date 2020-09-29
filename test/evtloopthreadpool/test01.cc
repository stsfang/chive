#include "chive/net/EventLoop.h"
#include "chive/net/EventLoopThread.h"
#include "chive/base/clog/chiveLog.h"


#include <stdio.h>
// #define NDEBUG
#include <assert.h>

void runInThread()
{
    CHIVE_LOG_DEBUG("run in thread...")
}

int main() 
{
    startLogPrint(nullptr);
    CHIVE_LOG_DEBUG("main() begin...");

    chive::net::EventLoopThread loopThread;
    chive::net::EventLoop* loop = loopThread.startLoop();
    CHIVE_LOG_DEBUG("main() run after...");

    loop->runInLoop(runInThread);
    loop->runAfter(2000, runInThread);
    // sleep(1);
    
    sleep(6);
    loop->quit();

    CHIVE_LOG_DEBUG("main() exit main...");
    
}