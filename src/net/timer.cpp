#include "src/net/timer.h"

namespace xraft {
namespace net {

std::atomic_int_fast64_t Timer::s_created_num;

void Timer::restart(util::Timestamp now) {
    if (repeat_) {
        expiration_ = util::add_time(now, interval_);
    } else {
        expiration_ = util::Timestamp::invalid();
    }
}

}
}