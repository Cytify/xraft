#include "src/net/socket_ops.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>  // snprintf
#include <strings.h>  // bzero
#include <sys/socket.h>
#include <unistd.h>

#include "src/log/logger.h"

namespace xraft {
namespace sockets {

namespace {
using SA = struct sockaddr;

const SA* sockaddr_cast(const struct sockaddr_in* addr) {
    return static_cast<const SA*>(reinterpret_cast<const void*>(addr));
}

SA* sockaddr_cast(struct sockaddr_in* addr) {
    return static_cast<SA*>(reinterpret_cast<void*>(addr));
}

}

int create_nonblocking_socket_or_abort() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        LOG_ERROR << "xraft::sockets::create_nonblocking_socket_or_abort";
        abort();
    }

    return sockfd;
}

void bind_or_abort(int sockfd, const struct sockaddr_in& addr) {
    int ret = ::bind(sockfd, sockaddr_cast(&addr), sizeof(addr));
    if (ret < 0) {
        LOG_ERROR << "xraft::sockets::bind_or_abort";
        abort();
    }
}

void listen_or_abort(int sockfd) {
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0) {
        LOG_ERROR << "xraft::sockets::listen_or_abort";
        abort();
    }
}

int accept(int sockfd, struct sockaddr_in* addr) {
    socklen_t addr_len = sizeof(*addr);
    int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0) {
        LOG_ERROR << "xraft::sockets::accept";
        abort();
    }

    return connfd;
}

void close(int sockfd) {
    int ret = ::close(sockfd);
    if (ret < 0) {
        LOG_ERROR << "xraft::sockets::close";
        abort();
    }
}

void to_host_port(char* buf, size_t len, const struct sockaddr_in& addr) {
    char host[INET_ADDRSTRLEN] = "INVALID";
    inet_ntop(AF_INET, &addr.sin_addr, host, sizeof(host));
    uint16_t port = xraft::sockets::network_to_host_16(addr.sin_port);
    snprintf(buf, len, "%s:%u", host, port);
}

void from_host_port(const char* ip, uint16_t port, struct sockaddr_in* addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = host_to_network_16(port);
    int ret = inet_pton(AF_INET, ip, &addr->sin_addr);
    if (ret < 0) {
        LOG_ERROR << "xraft::sockets::from_host_port";
    }
}

struct sockaddr_in get_local_addr(int sockfd) {
    struct sockaddr_in local_addr;
    bzero(&local_addr, sizeof(local_addr));
    socklen_t addr_len = sizeof(local_addr);
    if (getsockname(sockfd, sockaddr_cast(&local_addr), &addr_len) < 0) {
        LOG_ERROR << "sockets::get_local_addr";
    }
    return local_addr;
}

}
}