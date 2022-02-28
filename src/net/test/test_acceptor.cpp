#include <stdio.h>
#include <unistd.h>

#include "src/net/event_loop.h"
#include "src/net/acceptor.h"
#include "src/net/inet_address.h"
#include "src/net/socket_ops.h"

void new_conn(int sockfd, const xraft::net::InetAddress& peer_addr) {
    printf("new_conn(): accepted a new connection from %s\n", peer_addr.to_host_port().c_str());
    write(sockfd, "AKAKAKAA\n", 9);
    xraft::sockets::close(sockfd);
}

int main() {
    printf("main(): pid = %d\n", getpid());

    xraft::net::InetAddress listen_addr(9981);
    xraft::net::EventLoop loop;

    xraft::net::Acceptor acceptor(&loop, listen_addr);
    acceptor.set_new_conn_callback(new_conn);
    acceptor.listen();

    loop.loop();
}