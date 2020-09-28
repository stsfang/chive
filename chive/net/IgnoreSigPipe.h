#ifndef CHIVE_NET_IGNORESIGPIPE_H
#define CHIVE_NET_IGNORESIGPIPE_H

#include <signal.h>
#include "chive/base/clog/chiveLog.h"

namespace chive
{
namespace net
{
class IgnoreSigPipe
{
public:
    IgnoreSigPipe()
    {
        CHIVE_LOG_DEBUG("IgnoreSigPipe inited");
        ::signal(SIGPIPE, SIG_IGN);
    }
};

// 全局对象
extern IgnoreSigPipe initObj;
} // namespace net

} // namespace chive

#endif