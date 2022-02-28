#ifndef TIMER_H
#define TIMER_H

#include <atomic>

#include "src/util/noncopyable.h"
#include "src/util/timestamp.h"
#include "src/net/callback.cpp"

namespace xraft {
namespace net {

class Timer : util::Noncopyable {
public:
    Timer(TimerCallback cb, util::Timestamp when, double interval)
        : callback_(std::move(cb)), interval_(interval), repeat_(interval_ > 0.0),
          sequence_(++s_created_num), expiration_(when) {
    }

    void run() const {
        callback_();
    }

    util::Timestamp get_expiration() const {
        return expiration_;
    }

    bool get_repeat() const {
        return repeat_;
    }

    int64_t  get_sequence() const {
        return sequence_;
    }

    void restart(util::Timestamp now);

    static int64_t get_created_num() {
        return s_created_num.load();
    }

private:
    static std::atomic_int_fast64_t s_created_num;

    const TimerCallback callback_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;
    util::Timestamp expiration_;
};

}
}

#endif
