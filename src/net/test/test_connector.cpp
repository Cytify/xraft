#include <stdio.h>

#include "src/net/connector.h"
#include "src/net/event_loop.h"

xraft::net::EventLoop* g_loop;

void connect_callback(int sockfd) {
    printf("connected\n");
    g_loop->quit();
}

int main() {
    xraft::net::EventLoop loop;
    g_loop = &loop;

    xraft::net::InetAddress addr("127.0.0.1", 9981);
    xraft::net::ConnectorPtr connector(new xraft::net::Connector(&loop, addr));
    connector->set_new_connection_callback(connect_callback);
    connector->start();

    loop.loop();
}