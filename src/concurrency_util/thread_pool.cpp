#include "thread_pool.h"

namespace util {

ThreadPool::ThreadPool(const std::string& name)
    : name_(name), mutex_(), not_empty_(), not_full_(),
      running_() {
    
}

ThreadPool::~ThreadPool() {
    if (running_) {
        stop();
    }
}

void ThreadPool::start(int thread_num) {
    running_ = true;
    threads_.reserve(thread_num);
    for (int i = 0; i < thread_num; ++i) {
        char id[32];
        snprintf(id, sizeof(id), "%d", i + 1);
        threads_.emplace_back(std::make_shared<std::thread>(&ThreadPool::run_in_thread, this));
    }
}

void ThreadPool::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
        not_empty_.notify_all();
    }
    for (auto& thread : threads_) {
        thread->join();
    }
}

void ThreadPool::run(Task task) {
    if (threads_.empty()) {
        task();
    } else {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_) {
            return;
        }

        task_queue_.push_back(std::move(task));
        not_empty_.notify_one();
    }
}

void ThreadPool::run_in_thread() {
    while (running_) {
        Task task(take());
        if (task) {
            task();
        }
    }
}

ThreadPool::Task ThreadPool::take() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (running_ && task_queue_.empty()) {
        not_empty_.wait(lock);
    }
    Task task;
    if (!task_queue_.empty()) {
        task = task_queue_.front();
        task_queue_.pop_front();
    }

    return task;
}

}