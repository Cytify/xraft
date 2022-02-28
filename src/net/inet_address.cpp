#include "src/net/inet_address.h"

#include <string.h>

#include "socket_ops.h"

namespace xraft {
namespace net {

InetAddress::InetAddress(uint16_t port) {
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = xraft::sockets::host_to_network_32(INADDR_ANY);
    addr_.sin_port = xraft::sockets::host_to_network_16(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port) {
    bzero(&addr_, sizeof(addr_));
    xraft::sockets::from_host_port(ip.c_str(), port, &addr_);
}

std::string InetAddress::to_host_port() const {
    char buf[32];
    xraft::sockets::to_host_port(buf, sizeof(buf), addr_);

    return buf;
}

}
}