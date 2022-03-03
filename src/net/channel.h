#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <functional>

#include "src/util/noncopyable.h"
#include "src/util/timestamp.h"

namespace xraft {
namespace net {

class EventLoop;

/**
 * @brief channel 负责一个文件描述符的IO事件分发，但不会管理这个fd
 * channel 会将不同IO事件分发为不同回调函数，读、写等
 * 
 */
class Channel : util::Noncopyable {
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(util::Timestamp)>;

    Channel(EventLoop* loop, int fd);

    ~Channel();

    // 核心，根据revents，执行对应回调
    void handle_event(util::Timestamp receive_time);

    void set_read_callback(const ReadEventCallback& cb) {
        read_callback_ = cb;
    }

    void set_write_callback(const EventCallback& cb) {
        write_callback_ = cb;
    }

    void set_error_callback(const EventCallback& cb) {
        error_callback_ = cb;
    }

    void set_close_callback(const EventCallback& cb) {
        close_callback_ = cb;
    }

    int get_fd() const {
        return fd_;
    }

    int get_events() const {
        return events_;
    }

    void set_revents(int revents) {
        revents_ = revents;
    }

    bool is_none_event() const {
        return events_ == kNoneEvent;
    }

    int get_index() const {
        return index_;
    }

    void set_index(int index) {
        index_ = index;
    }

    void enable_read() {
        events_ |= kReadEvent;
        update();
    }
    
    void enable_write() {
        events_ |= kWriteEvent;
        update();
    }
    
    void disable_write() {
        events_ &= ~kWriteEvent;
        update();
    }

    void disable_all() {
        events_ = kNoneEvent;
        update();
    }

    EventLoop* onwer_loop() {
        return loop_;
    }

    bool is_writing() const {
        return events_ & kWriteEvent;
    }

private:
    void update();

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    const int fd_;
    int events_;
    int revents_;
    int index_;             // poller以vector形式存储channel，在这里记住下标

    ReadEventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback error_callback_;
    EventCallback close_callback_;

    bool event_handling_;
};

}
}

#endif