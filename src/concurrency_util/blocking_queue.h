#ifndef BLOCKING_QUEUE_H_
#define BLOCKING_QUEUE_H_

#include <mutex>
#include <condition_variable>
#include <deque>

#include "src/util/noncopyable.h"

namespace util {

template<typename T>
class BlockingQueue : Noncopyable {
public:
    using queue_type = std::deque<T>;

    BlockingQueue() : mutex_(), not_empty_(), queue_() {}

    void put(const T& x) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(x);
        not_empty_.notify_one();
    }

    void put(T&& x) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(std::move(x));
        not_empty_.notify_one();
    }

    T take() {
        std::unique_lock<std::mutex> lock(mutex_);
        while (queue_.empty()) {
            not_empty_.wait(lock);
        }
        T front(std::move(queue_.front()));
        queue_.pop_front();

        return front;
    }

    // 取出所有元素
    queue_type drain() {
        queue_type q;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            swap(q, queue_);
        }

        return q;
    }

private:
    std::mutex mutex_;
    std::condition_variable not_empty_;
    queue_type queue_;
};

}

#endif