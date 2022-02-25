#include "src/concurrency_util/thread_pool.h"
#include "src/util/current_thread.h"
#include "src/concurrency_util/count_down_latch.h"

#include <stdio.h>
#include <iostream>
#include <unistd.h>  // usleep

void print() {
    printf("tid=%d\n", util::current_thread::get_tid());
}

void print_string(const std::string& str) {
    std::cout << str << std::endl;
    usleep(100 * 1000);
}

void test() {
    std::cout << "Test ThreadPool" << std::endl;
    util::ThreadPool pool("MainThreadPool");
    pool.start(5);

    std::cout << "Adding" << std::endl;
    pool.run(print);
    pool.run(print);
    for (int i = 0; i < 100; ++i) {
        char buf[32];
        snprintf(buf, sizeof(buf), "task %d", i);
        pool.run(std::bind(print_string, std::string(buf)));
    }
    std::cout << "Done" << std::endl;

    util::CountDownLatch latch(1);
    pool.run(std::bind(&util::CountDownLatch::count_down, &latch));
    latch.wait();
    std::cout << "begin stop" << std::endl;
    pool.stop();
    std::cout << "test down" << std::endl;
}

void longTask(int num) {
    std::cout << "longTask " << num << std::endl;
    usleep(100*1000);
}

void test2() {
    std::cout << "Test ThreadPool by stoping early." << std::endl;
    util::ThreadPool pool("ThreadPool");
    pool.start(3);

    std::thread thread1([&pool]() {
        for (int i = 0; i < 20; ++i) {
            pool.run(std::bind(longTask, i));
        } 
    });

    usleep(5 * 100 * 1000);
    std::cout << "stop pool" << std::endl;
    pool.stop();  // early stop

    thread1.join();
    // run() after stop()
    pool.run(print);
    std::cout << "test2 Done" << std::endl;;
}

int main() {
    test();
    test2();
}
