#ifndef SOCKET_H_
#define SOCKET_H_

#include "src/util/noncopyable.h"

namespace xraft {
namespace net {

class InetAddress;

class Socket : util::Noncopyable {
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}

    ~Socket();

    // 如果地址被使用，abort
    void bind_address(const InetAddress& addr);

    // 如果地址被使用，abort
    void listen();

    /**
     * 成功，返回建立连接的套接字，被设置为non-blocking  close-on-exec，addr被设置
     * 失败，返回-1，不操作addr
     */
    int accept(InetAddress* peeraddr);

    // Enable/disable SO_REUSEADDR
    void set_reuse_addr(bool on);

    int get_fd() const {
        return sockfd_;
    }

private:
    const int sockfd_;
};

}
}

#endif