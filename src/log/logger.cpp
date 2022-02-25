#include "logger.h"

#include "src/util/current_thread.h"

namespace xlog {

__thread char t_time_str[64];

__thread time_t t_last_second;

const char* log_level_name[Logger::NUM_LOG_LEVELS] = {
    "INFO  ",
    "WARN  ",
    "ERROR ",
};

void default_output_func(const char* msg, size_t len) {
    fwrite(msg, 1, len, stdout);
}

Logger::OutputFunc g_output = default_output_func;

Logger::Logger(SourceFile file, int line, LogLevel level)
    : impl_(file, line, level) {
}

Logger::~Logger() {
    impl_.finish();
    const util::Buffer<util::kLargeBuffer>& buffer = stream().get_buffer();
    g_output(buffer.get_data(), buffer.get_length());
}

void Logger::set_output_func(OutputFunc func) {
    g_output = func;
}

Logger::Impl::Impl(const SourceFile& file, int line, LogLevel level)
    : time_(util::Timestamp::now()), stream_(), 
      level_(level), basename_(file), line_(line) {
    format_time();
    util::current_thread::get_tid();
    stream_.append(util::current_thread::get_tid_string(), util::current_thread::get_tid_string_length());
    stream_.append(log_level_name[level], 6);
}

void Logger::Impl::format_time() {
    int64_t micro_second = time_.get_micro_seconds_since_epoch();
    time_t second = static_cast<time_t>(micro_second / util::Timestamp::kMicroSecondsPerSecond);
    if (second != t_last_second) {
        t_last_second = second;
        struct tm tm_time;
        ::gmtime_r(&second, &tm_time);
        int len = snprintf(t_time_str, sizeof(t_time_str), "%4d%02d%02d %02d:%02d:%02d",
                           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                           tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    stream_.append(t_time_str, 17);
}

inline LogStream& operator<<(LogStream& s, const SourceFile& v) {
    s.append(v.get_data(), v.get_size());
    return s;
}

void Logger::Impl::finish() {
    stream_ << " - " << basename_ << ':' << line_ << '\n';
}

}