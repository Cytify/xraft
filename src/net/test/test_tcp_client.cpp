#include <stdio.h>
#include <unistd.h>
#include <string>

#include "src/net/event_loop.h"
#include "src/net/inet_address.h"
#include "src/net/tcp_client.h"

std::string message = "Hello";

void on_connection(const xraft::net::TcpConnectionPtr conn) {
    if (conn->is_connected()) {
        printf("on_connection(): new connection [%s] from %s\n",
               conn->get_name().c_str(),
               conn->get_peer_addr().to_host_port().c_str());
        conn->send(message);
    } else {
        printf("on_connection(): connection [%s] is down\n",
               conn->get_name().c_str());
    }
}

void on_message(const xraft::net::TcpConnectionPtr& conn, xraft::net::Buffer* buffer, util::Timestamp receive_time) {
    printf("on_message(): received %zd bytes from connection [%s] at %s\n",
           buffer->readable_bytes(), conn->get_name().c_str(),
           receive_time.to_format_string().c_str());

    printf("on_message(): [%s]\n", buffer->retrieve_as_string().c_str());
}

int main() {
    xraft::net::EventLoop loop;
    xraft::net::InetAddress server_addr("localhose", 9981);
    xraft::net::TcpClient client(&loop, server_addr);

    client.set_connection_callback(on_connection);
    client.set_message_callback(on_message);
    client.enable_retry();
    client.connect();

    loop.loop();
}
