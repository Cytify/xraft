#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#include <stdint.h>
#include <string>

namespace util {

class Timestamp {
public:
    static const int kMicroSecondsPerSecond = 1000 * 1000;

    Timestamp() : micro_seconds_since_epoch_(0) {}
    
    explicit Timestamp(int64_t seconds) : micro_seconds_since_epoch_(seconds) {}
    
    ~Timestamp() {}

    std::string to_string() const;

    std::string to_format_string(bool show_micro_second = true) const;

    void swap(Timestamp& that) {
        std::swap(this->micro_seconds_since_epoch_, that.micro_seconds_since_epoch_);
    }

    bool valid() const {
        return micro_seconds_since_epoch_ > 0;
    }

    int64_t get_micro_seconds_since_epoch() const {
        return micro_seconds_since_epoch_;
    }

    time_t get_seconds_since_epoch() const {
        return static_cast<time_t>(micro_seconds_since_epoch_ / kMicroSecondsPerSecond);
    }

    static Timestamp now();

    static Timestamp invalid() {
        return Timestamp();
    }

    static Timestamp from_unix_time(time_t t) {
        return from_unix_time(t, 0);
    }
    
    static Timestamp from_unix_time(time_t t, int micro_seconds) {
        return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + micro_seconds);
    }

private:
    int64_t micro_seconds_since_epoch_;
};

}

#endif