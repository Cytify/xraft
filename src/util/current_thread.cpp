#include "current_thread.h"

#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>

namespace util {
namespace current_thread {

__thread int cached_tid = 0;
__thread char tid_string[32];
__thread int tid_string_length = 6;
__thread const char* thread_name = "unkonw";

pid_t gettid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

void cache_tid() {
    if (cached_tid == 0) {
        cached_tid = gettid();
        tid_string_length = snprintf(tid_string, sizeof(tid_string), "%5d ", cached_tid);
    }
}

bool is_main_thread() {
    return get_tid() == ::getpid();
}

void sleepUsec(int64_t usec) {
    // struct timespec ts = { 0, 0 };
    // ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
    // ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);
    // ::nanosleep(&ts, NULL)
}


}
}