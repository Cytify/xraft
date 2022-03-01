#ifndef INET_ADDRESS_H_
#define INET_ADDRESS_H_

#include <netinet/in.h>
#include <string>

namespace xraft {
namespace net {

class InetAddress {
public:
    // for server listen addr
    explicit InetAddress(uint16_t port);

    InetAddress(const std::string& ip, uint16_t port);

    InetAddress(const struct sockaddr_in& addr) : addr_(addr) {
    }

    std::string to_host_port() const;

    const struct sockaddr_in& get_sock_addr_inet() const {
        return addr_;
    }

    void set_sock_addr_inet(const struct sockaddr_in& addr) {
        addr_ = addr;
    }

private:
    struct sockaddr_in addr_;

};

}
}

#endif