#ifndef CHIVE_BASE_TIMESTAMP_H
#define CHIVE_BASE_TIMESTAMP_H

#include "chive/base/copyable.h"
#include "chive/base/Types.h"


namespace chive
{
namespace base
{
///
/// Time stamp in UTC, in microseconds resolution
/// This class is immutable
/// It's recommended to pass it by value, since it's passed in register on x64.
///
class Timestamp : copyable
{
public:
    ///
    /// invalid timstamp
    ///
    Timestamp()
    : microSecondsSinceEpoch_(0)
    {}
    
    /// 
    /// specify a timestamp 
    /// 
    explicit Timestamp(int64_t microSecondsSinceEpochArg)
    : microSecondsSinceEpoch_(microSecondsSinceEpochArg)
    {}

    void swap(Timestamp& that)
    {
        std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
    }

    string toString() const;
    string toFormattedString(bool showMicroseconds = true) const;

    bool isValid() const {return microSecondsSinceEpoch_ > 0;}

    int64_t gerMicroSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }

    time_t secondsSinceEpoch() const 
    {
        return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    }

    static Timestamp now();

    static Timestamp invalidInstance() {return Timestamp(); }

    static Timestamp fromUnixTime(time_t t) {return fromUnixTime(t, 0);}
    static Timestamp fromUnixTime(time_t, int microseconds)
    {
        return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds; )
    }

    static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t microSecondsSinceEpoch_;
};

/// 
/// comparable operators
///
inline bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch_ < rhs.microSecondsSinceEpoch_;
}
inline bool operator<=(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch_ < rhs.microSecondsSinceEpoch_
            || lhs.microSecondsSinceEpoch_ == rhs.microSecondsSinceEpoch_;
}

inline bool operator>(Timestamp lhs, Timestamp rhs)
{
    return !(lhs.microSecondsSinceEpoch_ <= rhs.microSecondsSinceEpoch_);
}

inline bool operator>=(Timestamp lhs, Timestamp rhs)
{
    return !(lhs.microSecondsSinceEpoch_ < rhs.microSecondsSinceEpoch_);
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch_ == rhs.microSecondsSinceEpoch_;
}

///
/// gets time difference of two timestamps, result in seconds
///
inline double timeDifferece(Timestamp high, Timestamp low)
{
    int64_t diff = high.gerMicroSecondsSinceEpoch() - low.gerMicroSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

///
/// add seconds to given timestamp
///
inline Timestamp addTime(Timestamp timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.gerMicroSecondsSinceEpoch() + delta);
}

} // namespace base

} // namespace chive

#endif