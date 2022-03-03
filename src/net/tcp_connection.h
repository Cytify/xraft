#ifndef TCP_CONNECTION_H_
#define TCP_CONNECTION_H_

#include <memory>

#include "src/util/noncopyable.h"
#include "src/net/callback.h"
#include "src/net/inet_address.h"
#include "src/net/buffer.h"
#include "src/util/timestamp.h"

namespace xraft {
namespace net {

class Channel;
class EventLoop;
class Socket;

class TcpConnection : util::Noncopyable, 
                      public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop* loop, const std::string& name, int sockfd, 
                  const InetAddress& local_addr, const InetAddress& peer_addr);

    ~TcpConnection();

    // 接收到tcp连接时调用，只调用一次
    void connect_established();

    // tcp server从其map中移除时，调用
    void connect_destroyed();

    EventLoop* get_loop() const {
        return loop_;
    }

    const std::string& get_name() const {
        return name_;
    }

    const InetAddress& get_local_addr() const {
        return local_addr_;
    }

    const InetAddress& get_peer_addr() const {
        return peer_addr_;
    }

    bool is_connected() const {
        return state_ == Connected;
    }

    void set_connection_callback(const ConnectionCallback& cb) {
        conn_callback_ = cb;
    }

    void set_message_callback(const MessageCallback& cb) {
        msg_callback_ = cb;
    }

    void set_close_callback(const CloseCallback& cb) {
        close_callback_ = cb;
    }

    void set_write_complete_callback(const WriteCompleteCallback& cb) {
        write_complete_callback_ = cb;
    }

    void send(const std::string& message);

    void shutdown();

    void set_tcp_no_delay(bool on);

private:
    enum State {
        Connecting,
        Connected,
        Disconnecting,
        Disconnected,
    };

    void set_state(State s) {
        state_ = s;
    }

    void handle_read(util::Timestamp receive_time);
    void handle_write();
    void handle_close();
    void handle_error();

    void send_in_loop(const std::string& message);
    void shutdown_in_loop();

    EventLoop* loop_;
    std::string name_;
    State state_;
    std::shared_ptr<Socket> socket_;
    std::shared_ptr<Channel> channel_;
    InetAddress local_addr_;
    InetAddress peer_addr_;

    ConnectionCallback conn_callback_;
    MessageCallback msg_callback_;
    CloseCallback close_callback_;
    WriteCompleteCallback write_complete_callback_;         // 低水位回调

    Buffer input_buffer_;
    Buffer output_buffer_;
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

}
}

#endif