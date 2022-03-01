#include "src/net/tcp_connection.h"

#include <unistd.h>
#include <assert.h>

#include "src/log/logger.h"
#include "src/net/event_loop.h"
#include "src/net/channel.h"
#include "src/net/socket.h"
#include "src/net/socket_ops.h"

namespace xraft {
namespace net {

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, int sockfd, 
    const InetAddress& local_addr, const InetAddress& peer_addr) 
    : loop_(loop), name_(name), state_(Connecting), socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)), local_addr_(local_addr), peer_addr_(peer_addr)  {
    LOG_INFO << "TcpConnection::constructor[" << name_ << "] at" << this << " fd=" << sockfd;
    channel_->set_read_callback(std::bind(&TcpConnection::handle_read, this));
    channel_->set_write_callback(std::bind(&TcpConnection::handle_write, this));
    channel_->set_close_callback(std::bind(&TcpConnection::handle_close, this));
    channel_->set_error_callback(std::bind(&TcpConnection::handle_error, this));
}

TcpConnection::~TcpConnection() {
    LOG_INFO << "TcpConnection::destructor[" << name_ << "] at" << this << " fd=" << channel_->get_fd();
}

void TcpConnection::connect_established() {
    loop_->assert_in_loop_thread();
    assert(state_ == State::Connecting);
    set_state(State::Connected);
    channel_->enable_read();

    conn_callback_(shared_from_this());
}

void TcpConnection::connect_destroyed() {
    loop_->assert_in_loop_thread();
    assert(state_ == State::Connected);
    set_state(State::Disconnected);
    channel_->disable_all();
    conn_callback_(shared_from_this());

    loop_->remove_channel(channel_.get());
}

void TcpConnection::handle_read() {
    char buf[65535];
    ssize_t n = read(channel_->get_fd(), buf, sizeof(buf));
    if (n > 0) {
        msg_callback_(shared_from_this(), buf, n);
    } else if (n == 0) {
        handle_close();
    } else {
        handle_error();
    }
}

void TcpConnection::handle_write() {

}

void TcpConnection::handle_close() {
    loop_->assert_in_loop_thread();
    LOG_INFO << "TcpConnection::handle_close state = " << state_;
    assert(state_ == State::Connected);
    channel_->disable_all();
    close_callback_(shared_from_this());
}

void TcpConnection::handle_error() {
    int err = xraft::sockets::get_socket_error(channel_->get_fd());
    LOG_ERROR << "TcpConnection::handle_error [ " << name_ << "] - SO_ERROR = " << err;
}

}
}