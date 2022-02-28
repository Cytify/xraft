#include "src/net/channel.h"

#include <sys/poll.h>

#include "src/net/event_loop.h"
#include "src/log/logger.h"

namespace xraft {
namespace net {

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1) {
    
}

void Channel::handle_event() {
    if (revents_ & POLLNVAL) {
        LOG_ERROR << "Channel::handle_event() POLLNVAL";
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
}

void Channel::update() {
    loop_->update_channel(this);
}

}
}