#ifndef CHIVE_NET_CALLBACKS_H
#define CHIVE_NET_CALLBACKS_H

#include <functional>
#include <memory>

namespace chive
{
namespace net
{
using TimerCallback = std::function<void()>;

} // namespace net

} // namespace chive

#endif