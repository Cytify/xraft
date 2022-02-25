#include <sys/timerfd.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "src/net/event_loop.h"
#include "src/net/poller.h"
#include "src/net/channel.h"

xraft::net::EventLoop* g_loop;

void timeout() {
    printf("Timeout\n");
    g_loop->quit();
}

int main() {
    xraft::net::EventLoop loop;
    g_loop = &loop;

    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    xraft::net::Channel channel(&loop, timerfd);
    channel.set_read_callback(timeout);
    channel.enable_read();

    struct itimerspec how_long;
    bzero(&how_long, sizeof(how_long));
    how_long.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd, 0, &how_long, NULL);

    loop.loop();

    ::close(timerfd);
}
