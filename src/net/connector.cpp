#include "src/net/connector.h"

#include <assert.h>

#include "src/log/logger.h"
#include "src/net/event_loop.h"
#include "src/net/channel.h"
#include "src/net/socket_ops.h"

namespace xraft {
namespace net {

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& server_addr)
    : loop_(loop), server_addr_(server_addr), connect_(false), 
      state_(States::Disconnected), retry_delay_ms_(kInitRetryDelayMs) {
    LOG_INFO << "constructor [" << this << "]";
}

Connector::~Connector() {
    LOG_INFO << "destructor [" << this << "]";
    loop_->cancel(timer_id_);
    assert(!channel_);
}

void Connector::start() {
    connect_ = true;
    loop_->run_in_loop(std::bind(&Connector::start_in_loop, this));
}

void Connector::start_in_loop() {
    loop_->assert_in_loop_thread();
    assert(state_ == States::Disconnected);
    if (connect_) {
        connect();
    } else {
        LOG_INFO << "do not connect";
    }
}

void Connector::restart() {
    loop_->assert_in_loop_thread();
    set_state(States::Disconnected);
    retry_delay_ms_ = kInitRetryDelayMs;
    connect_ = true;
    start_in_loop();
}

void Connector::stop() {
    connect_ = false;
    loop_->cancel(timer_id_);
}

void Connector::connect() {
    int sockfd = xraft::sockets::create_nonblocking_socket_or_abort();
    int ret = xraft::sockets::connect(sockfd, server_addr_.get_sock_addr_inet());
    int saved_errno = (ret == 0) ? 0 : errno;
    switch (saved_errno) {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            connecting(sockfd);
            break;
        
        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry(sockfd);
            break;

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            LOG_ERROR << "connect error in Connector::start_in_loop " << saved_errno;
            xraft::sockets::close(sockfd);
            break;
        default:
            LOG_ERROR << "unexpected error error in Connector::start_in_loop " << saved_errno;
            xraft::sockets::close(sockfd);
            break;
    }
}

void Connector::connecting(int sockfd) {
    set_state(States::Connecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->set_write_callback(std::bind(&Connector::handle_write, this));
    channel_->set_error_callback(std::bind(&Connector::handle_error, this));
    channel_->enable_write();
}

void Connector::handle_write() {
    LOG_INFO << "Connector::handle_write " << state_;

    if (state_ == States::Connecting) {
        int sockfd = remove_and_reset_channel();
        int err = xraft::sockets::get_socket_error(sockfd);
        if (err) {
            LOG_WARN << "Connector::handle_write - SO_ERROR = " << err;
            retry(sockfd);
        } else if (xraft::sockets::is_self_connect(sockfd)) {
            LOG_WARN << "Connector::handle_write - self connect = " << err;
            retry(sockfd);           
        } else {
            set_state(States::Connected);
            if (connect_) {
                new_conn_callback_(sockfd);
            } else {
                xraft::sockets::close(sockfd);
            }
            
        }
    } else {
        assert(state_ == States::Disconnected);
    }
}

void Connector::handle_error() {
    LOG_INFO << "Connector::handle_error";
    assert(state_ == States::Connecting);

    int sockfd = remove_and_reset_channel();
    int err = xraft::sockets::get_socket_error(sockfd);
    LOG_INFO << "SO_ERROR = " << err;
    retry(sockfd);
}

void Connector::retry(int sockfd) {
    xraft::sockets::close(sockfd);
    set_state(States::Disconnected);
    if (connect_) {
        LOG_INFO << "Connector::retry - retry connecting to " << server_addr_.to_host_port()
                 << " in " << retry_delay_ms_ << " milliseconds";
        timer_id_ = loop_->run_after(retry_delay_ms_ / 1000, std::bind(&Connector::start_in_loop, this));
        retry_delay_ms_ = std::min(retry_delay_ms_ * 2, kMaxRetryDelayMs);
    } else {
        LOG_INFO << "do not connect";
    }
}

int Connector::remove_and_reset_channel() {
    channel_->disable_all();
    loop_->remove_channel(channel_.get());
    int sockfd = channel_->get_fd();
    loop_->run_in_loop(std::bind(&Connector::reset_channel, this));
    return sockfd;
}

void Connector::reset_channel() {
    channel_.reset();
}

}
}