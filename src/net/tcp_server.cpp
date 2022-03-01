#include "src/net/tcp_server.h"

#include <stdio.h>  // snprintf
#include <string>
#include <assert.h>

#include "src/log/logger.h"
#include "src/net/event_loop.h"
#include "src/net/acceptor.h"
#include "src/net/socket_ops.h"

namespace xraft {
namespace net {

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listen_addr)
    : loop_(loop), name_(listen_addr.to_host_port()), acceptor_(new Acceptor(loop, listen_addr)),
      started_(false), next_conn_id_(1) {
    acceptor_->set_new_conn_callback(std::bind(&TcpServer::new_connection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {

}

void TcpServer::start() {
    if (!started_) {
        started_ = true;
    }

    if (!acceptor_->get_listenning()) {
        loop_->run_in_loop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}   

void TcpServer::remove_connection(const TcpConnectionPtr& conn) {
    loop_->assert_in_loop_thread();
    LOG_INFO << "TcpServer::remove_connection [" << name_ << "] - connection " << conn->get_name();
    size_t n = connections_.erase(conn->get_name());
    assert(n == 1);
    loop_->queue_in_loop(std::bind(&TcpConnection::connect_destroyed, this));
}

void TcpServer::new_connection(int sockfd, const InetAddress& peer_addr) {
    loop_->assert_in_loop_thread();
    char buf[32];
    snprintf(buf, sizeof(buf), "#%d", next_conn_id_);
    ++next_conn_id_;
    std::string conn_name = name_ + buf;

    LOG_INFO << "TcpServer::new_connection [" << name_ << "] - new connection ["
             << conn_name << "] from " << peer_addr.to_host_port();
    InetAddress local_addr(xraft::sockets::get_local_addr(sockfd));

    TcpConnectionPtr conn(new TcpConnection(loop_, conn_name, sockfd, local_addr, peer_addr));
    connections_[conn_name] = conn;
    conn->set_connection_callback(conn_callback_);
    conn->set_message_callback(msg_callback_);
    conn->set_close_callback(std::bind(&TcpServer::remove_connection, this, std::placeholders::_1));
    conn->connect_established();
}

}
}