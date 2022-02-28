#include <stdio.h>
#include <unistd.h>

#include "src/net/event_loop.h"
#include "src/net/event_loop_thread.h"
#include "src/util/current_thread.h"

void run_in_thread() {
    printf("run_in_thread(): pid = %d, tid = %d\n", getpid(), util::current_thread::get_tid());
}

int main() {
    printf("main(): pid = %d, tid = %d\n", getpid(), util::current_thread::get_tid());

    xraft::net::EventLoopThread loop_thread;
    xraft::net::EventLoop* loop = loop_thread.start_loop();
    loop->run_in_loop(run_in_thread);
    sleep(1);
    loop->run_after(2, run_in_thread);
    sleep(3);

    printf("exit main().\n");
}