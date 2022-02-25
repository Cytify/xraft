#ifndef COUNT_DOWN_LATCH_
#define COUNT_DOWN_LATCH_

#include <mutex>
#include <condition_variable>

#include "src/util/noncopyable.h"

namespace util {

class CountDownLatch : Noncopyable {
public:
    explicit CountDownLatch(int count) : mutex_(), cond_(), count_(count) {}

    void wait();

    void count_down();

    int get_count() const;

private:
    mutable std::mutex mutex_;
    std::condition_variable cond_;
    int count_;
};

}

#endif 