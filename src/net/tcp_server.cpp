#include "src/net/tcp_server.h"

#include <stdio.h>  // snprintf
#include <string>
#include <assert.h>

#include "src/log/logger.h"
#include "src/net/event_loop.h"
#include "src/net/acceptor.h"
#include "src/net/socket_ops.h"
#include "src/net/event_loop_thread_pool.h"

namespace xraft {
namespace net {

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listen_addr)
    : loop_(loop), name_(listen_addr.to_host_port()), acceptor_(new Acceptor(loop, listen_addr)),
      thread_pool_(new EventLoopThreadPool(loop, "")), started_(false), next_conn_id_(1) {
    acceptor_->set_new_conn_callback(std::bind(&TcpServer::new_connection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {

}

void TcpServer::start() {
    if (!started_) {
        started_ = true;
        thread_pool_->start();
    }

    if (!acceptor_->get_listenning()) {
        loop_->run_in_loop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}   


void TcpServer::set_thread_num(int thread_num) {
    assert(thread_num >= 0);
    thread_pool_->set_thread_num(thread_num);
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

    EventLoop* io_loop = thread_pool_->get_next_loop();

    TcpConnectionPtr conn(new TcpConnection(io_loop, conn_name, sockfd, local_addr, peer_addr));
    connections_[conn_name] = conn;
    conn->set_connection_callback(conn_callback_);
    conn->set_message_callback(msg_callback_);
    conn->set_close_callback(std::bind(&TcpServer::remove_connection, this, std::placeholders::_1));
    conn->set_write_complete_callback(write_complete_callback_);
    io_loop->run_in_loop(std::bind(&TcpConnection::connect_established, conn));
}

void TcpServer::remove_connection(const TcpConnectionPtr& conn) {
    loop_->run_in_loop(std::bind(&TcpServer::remove_connection_in_loop, this, conn));
}

void TcpServer::remove_connection_in_loop(const TcpConnectionPtr& conn) {
    loop_->assert_in_loop_thread();
    LOG_INFO << "TcpServer::remove_connection_in_loop [" << name_
             << "] - connection " << conn->get_name();
    ssize_t n = connections_.erase(conn->get_name());
    assert(n == 1);
    EventLoop* io_loop = conn->get_loop();
    io_loop->queue_in_loop(std::bind(&TcpConnection::connect_destroyed, conn));
}

}
}