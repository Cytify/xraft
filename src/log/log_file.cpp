#include "src/log/log_file.h"

#include <time.h>

namespace xlog {

LogFile::LogFile(const std::string& basename, off_t roll_size, int flush_interval, int check_number)
    : basename_(basename), roll_size_(roll_size), flush_interval_(flush_interval), check_number_(check_number),
      written_count_(0), mutex_(), current_period(0), last_roll_(0), last_flush_(0) {
    roll_file();
}

void LogFile::append(const char* buf, size_t len) {
    std::lock_guard<std::mutex> lock(mutex_);
    append_unlock(buf, len);
}

void LogFile::flush() {
    file_->flush();
}

bool LogFile::roll_file() {
    time_t now = 0;
    std::string filename = get_log_file_name(basename_, &now);
    time_t now_period = now / kRollPerSeconds_ * kRollPerSeconds_;
    if (now > last_roll_) {
        last_roll_ = now;
        last_flush_ = now;
        current_period = now_period;
        file_.reset(new util::AppendFile(filename));
        
        return true;
    }
    return false;
}

void LogFile::append_unlock(const char* buf, size_t len) {
    file_->append(buf, len);

    if (file_->get_written_bytes() > roll_size_) {
        roll_file();
    } else {
        ++written_count_;
        if (written_count_ >= check_number_) {
            written_count_ = 0;
            time_t now = time(NULL);
            time_t now_period = now / kRollPerSeconds_ * kRollPerSeconds_;
            if (now_period != current_period) {
                roll_file();
            } else if (now - last_flush_ > flush_interval_) {
                last_flush_ = now;
                file_->flush();
            }
        }
    }
}

std::string LogFile::get_log_file_name(const std::string& basename, time_t* now) {
    std::string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;

    char timebuf[32];
    struct tm tm_time;
    *now = time(NULL);
    gmtime_r(now, &tm_time);
    strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S", &tm_time);
    filename += timebuf;

    filename += ".log";

    return filename;
}

}