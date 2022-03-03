#include <stdio.h>
#include <unistd.h>

#include "src/net/event_loop.h"
#include "src/net/tcp_server.h"
#include "src/net/inet_address.h"
#include "src/util/timestamp.h"
#include "src/net/buffer.h"

void on_connection(const xraft::net::TcpConnectionPtr conn) {
    if (conn->is_connected()) {
        printf("on_connection(): new connection [%s] from %s\n",
               conn->get_name().c_str(),
               conn->get_peer_addr().to_host_port().c_str());
    } else {
        printf("on_connection(): connection [%s] is down\n",
               conn->get_name().c_str());
    }
}

void on_message(const xraft::net::TcpConnectionPtr& conn, xraft::net::Buffer* buffer, util::Timestamp receive_time) {
    printf("on_message(): received %zd bytes from connection [%s] at %s\n",
           buffer->readable_bytes(), conn->get_name().c_str(),
           receive_time.to_format_string().c_str());

    conn->send(buffer->retrieve_as_string());
}

int main() {
    printf("main(): pid = %d\n", getpid());

    xraft::net::InetAddress listen_addr(9981);
    xraft::net::EventLoop loop;

    xraft::net::TcpServer server(&loop, listen_addr);
    server.set_connection_callback(on_connection);
    server.set_message_callback(on_message);
    server.start();

    loop.loop();
}