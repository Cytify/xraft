#include "count_down_latch.h"

namespace util {

void CountDownLatch::wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (count_ > 0) {
        cond_.wait(lock);
    }
}

void CountDownLatch::count_down() {
    std::lock_guard<std::mutex> lock(mutex_);
    --count_;
    if (count_ == 0) {
        cond_.notify_all();
    }
}

int CountDownLatch::get_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return count_;
}

}
