#ifndef CHIVE_BASE_COPYABLE_H
#define CHIVE_BASE_COPYABLE_H

namespace chive
{
class copyable 
{
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};
} // namespace chive
#endif