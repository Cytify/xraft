#ifndef LOG_FILE_H_
#define LOG_FILE_H_

#include <memory>
#include <string>
#include <mutex>

#include "src/util/append_file.h"

namespace xlog {

class LogFile {
public:
    LogFile(const std::string& basename, off_t roll_size, int flush_interval = 3, int check_number = 1024);

    ~LogFile() = default;

    void append(const char* buf, size_t len);

    void flush();

    bool roll_file();

private:
    void append_unlock(const char* buf, size_t len);

    std::string get_log_file_name(const std::string& basename, time_t* now);

    const static int kRollPerSeconds_ = 60 * 60 * 24;

    const std::string basename_;
    const off_t roll_size_;
    const int flush_interval_;
    const int check_number_;        // 写入n次后检查

    int written_count_;

    std::mutex mutex_;
    time_t current_period;
    time_t last_roll_;
    time_t last_flush_;
    std::unique_ptr<util::AppendFile> file_;
};

}

#endif