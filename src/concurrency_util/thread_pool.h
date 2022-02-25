#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <string>
#include <deque>
#include <vector>

#include "src/util/noncopyable.h"

namespace util {

class ThreadPool : Noncopyable {
public:
    using Task = std::function<void ()>;

    explicit ThreadPool(const std::string& name = std::string("ThreadPool"));
    
    ~ThreadPool();

    void start(int thread_num);

    void stop();

    void run(Task task);

private:
    void run_in_thread();

    Task take();

    std::string name_;
    mutable std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    bool running_;
    std::vector<std::shared_ptr<std::thread>> threads_;
    std::deque<Task> task_queue_;
};

}

#endif