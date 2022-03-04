#include "src/net/tcp_client.h"

#include <stdio.h>

#include "src/log/logger.h"
#include "src/net/connector.h"
#include "src/net/event_loop.h"
#include "src/net/socket_ops.h"

namespace xraft {
namespace net {

namespace {

void remove_connection_inner(EventLoop* loop, const TcpConnectionPtr& conn) {
    loop->queue_in_loop(std::bind(&TcpConnection::connect_destroyed, conn));
}

}

TcpClient::TcpClient(EventLoop* loop, const InetAddress& server_addr)
    : loop_(loop), connector_(new Connector(loop, server_addr)), retry_(false),
      connect_(true), next_conn_id_(1) {
    connector_->set_new_connection_callback(std::bind(&TcpClient::new_connection, this, std::placeholders::_1));
    LOG_INFO << "TcpClient::TcpClient [" << this << "] - connector" << connector_.get();
}

TcpClient::~TcpClient() {
    LOG_INFO << "TcpClient::~TcpClient [" << this << "] - connector" << connector_.get();
    TcpConnectionPtr conn;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        conn = connection_;
    }

    if (conn) {
        CloseCallback cb = std::bind(&remove_connection_inner, loop_, std::placeholders::_1);
        loop_->run_in_loop(std::bind(&TcpConnection::set_close_callback, conn, cb));
    } else {
        connector_->stop();
    }
}

void TcpClient::connect() {
    LOG_INFO << "TcpClient::connect[" << this << "] - connecting to "
             << connector_->get_server_address().to_host_port();
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect() {
    connect_ = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (connection_) {
            connection_->shutdown();
        }
    }
}

void TcpClient::stop() {
    connect_ = false;
    connector_->stop();
}

void TcpClient::new_connection(int sockfd) {
    loop_->assert_in_loop_thread();
    InetAddress peer_addr(xraft::sockets::get_peer_addr(sockfd));
    char buf[32];
    snprintf(buf, sizeof(buf), ":%s#%d", peer_addr.to_host_port().c_str(), next_conn_id_);
    ++next_conn_id_;
    std::string conn_name = buf;

    InetAddress local_addr(xraft::sockets::get_local_addr(sockfd));

    TcpConnectionPtr conn(new TcpConnection(loop_, conn_name, sockfd, local_addr, peer_addr));
    conn->set_connection_callback(conn_callback_);
    conn->set_message_callback(msg_callback_);
    conn->set_close_callback(std::bind(&TcpClient::remove_connection, this, std::placeholders::_1));
    conn->set_write_complete_callback(write_complete_callback_);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_ = conn;
    }
    conn->connect_established();
}

void TcpClient::remove_connection(const TcpConnectionPtr& conn) {
    loop_->assert_in_loop_thread();
    assert(loop_ == conn->get_loop());

    {
        std::lock_guard<std::mutex> lock(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }

    loop_->queue_in_loop(std::bind(&TcpConnection::connect_destroyed, conn));
    if (retry_ && connect_) {
        LOG_INFO << "TcpClient::connect [" << this << "] - Reconnecting to "
                 << connector_->get_server_address().to_host_port();
        connector_->restart();
    }
}

}
}