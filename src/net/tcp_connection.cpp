#include "src/net/tcp_connection.h"

#include <unistd.h>
#include <assert.h>

#include "src/log/logger.h"
#include "src/net/event_loop.h"
#include "src/net/channel.h"
#include "src/net/socket.h"

namespace xraft {
namespace net {

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, int sockfd, 
    const InetAddress& local_addr, const InetAddress& peer_addr) 
    : loop_(loop), name_(name), state_(Connecting), socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)), local_addr_(local_addr), peer_addr_(peer_addr)  {
    LOG_INFO << "TcpConnection::constructor[" << name_ << "] at" << this << " fd=" << sockfd;
    channel_->set_read_callback(std::bind(&TcpConnection::handle_read, this));
}

TcpConnection::~TcpConnection() {
    LOG_INFO << "TcpConnection::destructor[" << name_ << "] at" << this << " fd=" << channel_->get_fd();
}

void TcpConnection::connect_established() {
    loop_->assert_in_loop_thread();
    assert(state_ == Connecting);
    set_state(Connected);
    channel_->enable_read();

    conn_callback_(shared_from_this());
}

void TcpConnection::handle_read() {
    char buf[65535];
    ssize_t n = read(channel_->get_fd(), buf, sizeof(buf));
    msg_callback_(shared_from_this(), buf, n);
}

}
}