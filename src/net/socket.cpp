#include "src/net/socket.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <strings.h>    // bzero

#include "src/net/inet_address.h"
#include "src/net/socket_ops.h"

namespace xraft {
namespace net {

Socket::~Socket() {
    xraft::sockets::close(sockfd_);
}

void Socket::bind_address(const InetAddress& addr) {
    xraft::sockets::bind_or_abort(sockfd_, addr.get_sock_addr_inet());
}

void Socket::listen() {
    xraft::sockets::listen_or_abort(sockfd_);
}

int Socket::accept(InetAddress* peeraddr) {
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    int connfd = xraft::sockets::accept(sockfd_, &addr);
    if (connfd >= 0) {
        peeraddr->set_sock_addr_inet(addr);
    }

    return connfd;
}

void Socket::set_reuse_addr(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

}
}