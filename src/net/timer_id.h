#ifndef TIMER_ID_H_
#define TIMER_ID_H_

#include <stdint.h>

namespace xraft {
namespace net {

class Timer;

class TimerId {
public:
    TimerId() : timer_(nullptr), sequence_(0) {

    }

    TimerId(Timer* timer, int64_t seq)
        : timer_(timer), sequence_(seq) {

    }

private:
    Timer* timer_;
    int64_t sequence_;
};

}
}

#endif