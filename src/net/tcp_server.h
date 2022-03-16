#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include <map>

#include "src/util/noncopyable.h"
#include "src/net/tcp_connection.h"

namespace xraft {
namespace net {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer : util::Noncopyable {
public:
    TcpServer(EventLoop* loop, const InetAddress& listen_addr);

    ~TcpServer();

    // 启动服务器
    void start();

    void set_connection_callback(const ConnectionCallback& cb) {
        conn_callback_ = cb;
    }

    void set_message_callback(const MessageCallback& cb) {
        msg_callback_ = cb;
    }

    void set_write_complete_callback(const WriteCompleteCallback& cb) {
        write_complete_callback_ = cb;
    }

    void set_thread_num(int thread_num);

private:
    // 新连接到来时调用
    void new_connection(int sockfd, const InetAddress& peer_addr);

    void remove_connection(const TcpConnectionPtr& conn);

    void remove_connection_in_loop(const TcpConnectionPtr& conn);

    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    EventLoop* loop_;
    const std::string name_;
    std::shared_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> thread_pool_;
    ConnectionCallback conn_callback_;
    MessageCallback msg_callback_;
    WriteCompleteCallback write_complete_callback_;
    bool started_;
    int next_conn_id_;
    ConnectionMap connections_;
};

}
}

#endif
