#ifndef ACCEPTOR_H_
#define ACCEPTOR_H_

#include <functional>

#include "src/util/noncopyable.h"
#include "src/net/socket.h"
#include "src/net/channel.h"

namespace xraft {
namespace net {

class EventLoop;
class InetAddress;

class Acceptor : util::Noncopyable {
public:
    using NewConnCallback = std::function<void(int sockfd, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& listen_addr);

    void listen();

    bool get_listenning() const {
        return listenning_;
    }

    void set_new_conn_callback(const NewConnCallback& cb) {
        new_conn_callback_ = cb;
    }

private:
    // listen到新连接来临，调用handle_read回调
    void handle_read();

    EventLoop* loop_;
    Socket accept_socket_;
    Channel accept_channel_;
    NewConnCallback new_conn_callback_;
    bool listenning_;
};

}
}

#endif