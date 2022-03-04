#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

#include <atomic>
#include <mutex>

#include "src/util/noncopyable.h"
#include "src/net/tcp_connection.h"

namespace xraft {
namespace net {

class Connector;
using ConnectorPtr = std::shared_ptr<Connector>;

class TcpClient : util::Noncopyable {
public:
    TcpClient(EventLoop* loop, const InetAddress& server_addr);

    ~TcpClient();

    TcpConnectionPtr get_connection() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return connection_;
    }

    bool get_retry() const {
        return retry_;
    }

    void enable_retry() { 
        retry_ = true; 
    }

    void set_connection_callback(const ConnectionCallback& cb) {
        conn_callback_ = cb;
    }

    void set_message_callback(const MessageCallback& cb) {
        msg_callback_ = cb;
    }

    void set_write_complete_callback(const WriteCompleteCallback& cb) {
        write_complete_callback_ = cb;
    }

    void connect();

    void disconnect();

    void stop();

private:
    void new_connection(int sockfd);

    void remove_connection(const TcpConnectionPtr& conn);

    EventLoop* loop_;
    ConnectorPtr connector_;
    
    ConnectionCallback conn_callback_;
    MessageCallback msg_callback_;
    WriteCompleteCallback write_complete_callback_;

    std::atomic_bool retry_;
    std::atomic_bool connect_;
    int next_conn_id_;
    mutable std::mutex mutex_;
    TcpConnectionPtr connection_;
};

}
}

#endif