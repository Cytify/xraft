#include "timestamp.h"

#include <inttypes.h>  
#include <sys/time.h>
#include <stdio.h>

namespace util {

std::string Timestamp::to_string() const {
    char buf[32] = {0};
    int64_t seconds = micro_seconds_since_epoch_ / kMicroSecondsPerSecond;
    int64_t micro_seconds = micro_seconds_since_epoch_ % kMicroSecondsPerSecond;
    snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64 "", seconds, micro_seconds);
    
    return buf;
}

std::string Timestamp::to_format_string(bool show_micro_second) const {
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(micro_seconds_since_epoch_ / kMicroSecondsPerSecond);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    if (show_micro_second) {
        int micro_seconds = static_cast<int>(micro_seconds_since_epoch_ % kMicroSecondsPerSecond);
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d", 
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, micro_seconds);
    } else {
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }

    return buf;
}

Timestamp Timestamp::now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return Timestamp(tv.tv_sec * kMicroSecondsPerSecond + tv.tv_usec);
}

}