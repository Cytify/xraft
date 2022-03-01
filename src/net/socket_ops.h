#ifndef SOCKET_OPS_H_
#define SOCKET_OPS_H_

#include <arpa/inet.h>
#include <endian.h>

namespace xraft {
namespace sockets {

inline uint64_t host_to_network_64(uint64_t host64) {
    return htobe64(host64);
}

inline uint32_t host_to_network_32(uint32_t host32) {
    return htonl(host32);
}

inline uint16_t host_to_network_16(uint16_t host16) {
    return htons(host16);
}

inline uint64_t network_to_host_64(uint64_t net64) {
    return be64toh(net64);
}

inline uint32_t network_to_host_32(uint32_t net32) {
    return ntohl(net32);
}

inline uint16_t network_to_host_16(uint16_t net16) {
    return ntohs(net16);
}

int create_nonblocking_socket_or_abort();

void bind_or_abort(int sockfd, const struct sockaddr_in& addr);

void listen_or_abort(int sockfd);

int accept(int sockfd, struct sockaddr_in* addr);

void close(int sockfd);

void to_host_port(char* buf, size_t len, const struct sockaddr_in& addr);

void from_host_port(const char* ip, uint16_t port, struct sockaddr_in* addr);

struct sockaddr_in get_local_addr(int sockfd);

int get_socket_error(int sockfd);

}
}

#endif