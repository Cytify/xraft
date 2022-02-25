#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>

#include "src/log/async_log.h"
#include "src/log/logger.h"
#include "src/log/log_file.h"

off_t kRollSize = 500 * 1000 * 1000;

xlog::AsyncLog* g_async_log = nullptr;

void output_func(const char* buf, size_t len) {
    g_async_log->append(buf, len);
}

void bench(bool long_log) {
    xlog::Logger::set_output_func(output_func);

    int cnt = 0;
    const int batch = 1000;
    std::string empty = " ";
    std::string long_str(3000, 'X');
    long_str += " ";

    for (int t = 0; t < 30; ++t) {
        util::Timestamp start = util::Timestamp::now();
        for (int i = 0; i < batch; ++i) {
            LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
                     << (long_log ? long_str : empty)
                     << cnt;
            ++cnt;
        }
        
        util::Timestamp end = util::Timestamp::now();
        double diff = 1.0 * (end.get_micro_seconds_since_epoch() - start.get_micro_seconds_since_epoch()) / util::Timestamp::kMicroSecondsPerSecond;
        printf("%f\n", diff * 1000000 / batch);
        struct timespec ts = { 0, 500 * 1000 * 1000 };
        nanosleep(&ts, NULL);
    }
}

int main(int argc, char* argv[]) {
    {
        // set max virtual memory to 2GB.
        size_t gb = 1000 * 1024 * 1024;
        rlimit rl = { 2 * gb, 2 * gb };
        setrlimit(RLIMIT_AS, &rl);
    }

    printf("pid = %d\n", getpid());
    char name[256] = { '\0' };
    strncpy(name, argv[0], sizeof(name) - 1);
    xlog::AsyncLog log(basename(name), kRollSize);
    log.start();
    g_async_log = &log;

    bool longLog = argc > 1;
    bench(longLog);
}