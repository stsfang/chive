#ifndef CHIVE_BASE_NONCOPYABLE_H
#define CHIVE_BASE_NONCOPYABLE_H

namespace chive
{
class noncopyable 
{
    public:
        noncopyable(const noncopyable&) = delete;
        void operator=(const noncopyable&) = delete;

    protected:
        noncopyable() = default;
        ~noncopyable() = default;
};
} // namespace chive
#endif