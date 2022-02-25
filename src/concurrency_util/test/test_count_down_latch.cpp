#include <iostream>
#include <thread>

#include "src/concurrency_util/count_down_latch.h"

util::CountDownLatch g_latch(3);

void count_down() {
    g_latch.count_down();
}

void wait_latch() {
    std::cout << "latch wait begin" << std::endl;
    g_latch.wait();
    std::cout << "latch wait over" << std::endl;
}

int main() {
    std::thread t1(wait_latch);
    std::thread t2(count_down);
    std::thread t3(count_down);
    std::thread t4(count_down);
    
    t1.join();
    t2.join();
    t3.join();
    t4.join();
}