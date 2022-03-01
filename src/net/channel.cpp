#include "src/net/channel.h"

#include <sys/poll.h>
#include <assert.h>

#include "src/net/event_loop.h"
#include "src/log/logger.h"

namespace xraft {
namespace net {

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1),
      event_handling_(false) {
    
}

Channel::~Channel() {
    assert(!event_handling_);
}

void Channel::handle_event() {
    event_handling_ = true;
    if (revents_ & POLLNVAL) {
        LOG_ERROR << "Channel::handle_event() POLLNVAL";
    }    

    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
        LOG_WARN << "Channel::handle_event() POLLHUP";
        if (close_callback_) {
            close_callback_();
        }
    }

    if (revents_ & (POLLERR | POLLNVAL)) {
        if (error_callback_) {
            error_callback_();
        }
    }

    if (revents_ & (POLLIN || POLLPRI || POLLRDHUP)) {
        if (read_callback_) {
            read_callback_();
        }
    }

    if (revents_ & (POLLOUT)) {
        if (write_callback_) {
            write_callback_();
        }
    }

    event_handling_ = false;
}

void Channel::update() {
    loop_->update_channel(this);
}

}
}