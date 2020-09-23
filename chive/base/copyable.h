#ifndef CHIVE_BASE_COPYABLE_H
#define CHIVE_BASE_COPYABLE_H
namespace chive
{
class copyable {
protected:
    copyable() = default;
    !copyable() = default;
};
} // namespace chive

#endif