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
    channel_->set_read_callback(std::bind(&TcpConnection::handle_read, this, std::placeholders::_1));
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
    assert(state_ == State::Connected || state_ == State::Disconnecting);
    set_state(State::Disconnected);
    channel_->disable_all();
    conn_callback_(shared_from_this());

    loop_->remove_channel(channel_.get());
}

void TcpConnection::send(const std::string& message) {
    if (state_ == State::Connected) {
        if (loop_->is_in_loop_thread()) {
            send_in_loop(message);
        } else {
            loop_->run_in_loop(std::bind(&TcpConnection::send_in_loop, this, message));
        }
    }
}

void TcpConnection::send_in_loop(const std::string& message) {
    loop_->assert_in_loop_thread();
    ssize_t n_wrote = 0;
    // 如果输出队列为空，直接写
    if (!channel_->is_writing() && output_buffer_.readable_bytes() == 0) {
        n_wrote = write(channel_->get_fd(), message.c_str(), message.size());
        if (n_wrote >= 0) {
            if (static_cast<size_t>(n_wrote) < message.size()) {
                LOG_INFO << "I am going to write more data";
            } else if (write_complete_callback_) {  
                loop_->queue_in_loop(std::bind(write_complete_callback_, shared_from_this()));
            }
        } else {
            n_wrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_ERROR << "TcpConnection::send_in_loop";
                abort();
            }
        }
    }

    assert(n_wrote >= 0);
    if (static_cast<size_t>(n_wrote) < message.size()) {
        output_buffer_.append(message.data() + n_wrote, message.size() - n_wrote);
        if (!channel_->is_writing()) {
            channel_->enable_write();
        }
    }

}

void TcpConnection::shutdown() {
    if (state_ == State::Connected) {
        set_state(State::Disconnecting);
        loop_->run_in_loop(std::bind(&TcpConnection::shutdown_in_loop, this));
    }
}

void TcpConnection::shutdown_in_loop() {
    loop_->assert_in_loop_thread();
    if (!channel_->is_writing()) {
        socket_->shutdown_write();
    }
}

void TcpConnection::set_tcp_no_delay(bool on) {
    socket_->set_tcp_no_delay(on);
}

void TcpConnection::handle_read(util::Timestamp receive_time) {
    ssize_t n = input_buffer_.read_fd(channel_->get_fd());
    if (n > 0) {
        msg_callback_(shared_from_this(), &input_buffer_, receive_time);
    } else if (n == 0) {
        handle_close();
    } else {
        handle_error();
    }
}

void TcpConnection::handle_write() {
    loop_->assert_in_loop_thread();
    if (channel_->is_writing()) {
        ssize_t n = write(channel_->get_fd(), output_buffer_.peek(), output_buffer_.readable_bytes());
        if (n > 0) {
            output_buffer_.retrieve(n);
            if (output_buffer_.readable_bytes() == 0) {
                channel_->disable_write();
                if (write_complete_callback_) {
                    loop_->queue_in_loop(std::bind(write_complete_callback_, shared_from_this()));
                }
                if (state_ == State::Disconnecting) {
                    shutdown_in_loop();
                }
            } else {
                LOG_INFO << "I am going to write more data";
            }
        } else {
            LOG_ERROR << "TcpConnection::send_in_loop";
            abort();
        }
    } else {
        LOG_INFO << "connection is down, no more writing";
    }
}

void TcpConnection::handle_close() {
    loop_->assert_in_loop_thread();
    LOG_INFO << "TcpConnection::handle_close state = " << state_;
    assert(state_ == State::Connected || state_ == State::Disconnecting);
    channel_->disable_all();
    close_callback_(shared_from_this());
}

void TcpConnection::handle_error() {
    int err = xraft::sockets::get_socket_error(channel_->get_fd());
    LOG_ERROR << "TcpConnection::handle_error [ " << name_ << "] - SO_ERROR = " << err;
}

}
}