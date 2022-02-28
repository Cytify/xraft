#include "src/net/acceptor.h"

#include "src/log/logger.h"
#include "src/net/event_loop.h"
#include "src/net/inet_address.h"
#include "src/net/socket_ops.h"

namespace xraft {
namespace net {

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listen_addr)
    : loop_(loop), accept_socket_(xraft::sockets::create_nonblocking_socket_or_abort()),
      accept_channel_(loop, accept_socket_.get_fd()), listenning_(false)  {
    accept_socket_.set_reuse_addr(true);
    accept_socket_.bind_address(listen_addr);
    accept_channel_.set_read_callback(std::bind(&Acceptor::handle_read, this));
}

void Acceptor::listen() {
    loop_->assert_in_loop_thread();
    listenning_ = true;
    accept_socket_.listen();
    accept_channel_.enable_read();
}

void Acceptor::handle_read() {
    loop_->assert_in_loop_thread();
    InetAddress peer_addr(0);
    int connfd = accept_socket_.accept(&peer_addr);
    if (connfd >= 0) {
        if (new_conn_callback_) {
            new_conn_callback_(connfd, peer_addr);
        } else {
            xraft::sockets::close(connfd);
        }
    }
}

}
}