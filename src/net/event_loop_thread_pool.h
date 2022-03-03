#ifndef EVENT_LOOP_THREAD_POOL_H_
#define EVENT_LOOP_THREAD_POOL_H_

#include <functional>
#include <memory>
#include <vector>
#include <string>

#include "src/util/noncopyable.h"

namespace xraft {
namespace net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : util::Noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* base_loop, const std::string& name);

    ~EventLoopThreadPool();

    bool get_started() const {
        return started_;
    }

    const std::string& get_name() const {
        return name_;
    }

    void set_thread_num(int thread_num) {
        thread_num_ = thread_num;
    }

    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    EventLoop* get_next_loop();

    EventLoop* get_loop_for_hash(size_t hash_code);

    std::vector<EventLoop*> get_all_loops();

private:
    EventLoop* base_loop_;
    std::string name_;
    bool started_;
    int thread_num_;
    size_t next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;

};

}
}

#endif