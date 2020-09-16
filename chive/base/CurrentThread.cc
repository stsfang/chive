#include "chive/base/CurrentThread.h"

// using namespace chive;

#include <cstdio>
#include <sys/syscall.h>    /* For SYS_xxx definitions */

namespace chive
{
namespace CurrentThread
{
// 必须在与 .h 相同的namespace 下定义，否则导致二义性
// 即，编译器认不出是 .cc 的 还是 .h 的
// 定义 extern __thread
__thread int t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 6;
__thread const char* t_threadName = "unknown";
static_assert(std::is_same<int, pid_t>::value, "pid_t should be int");

void cachedTid() {
    if(t_cachedTid == 0) {
        t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
        t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
    }
}    
} // namespace CurrentThread

} // namespace chive

