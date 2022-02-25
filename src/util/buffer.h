#ifndef BUFFER_H_
#define BUFFER_H_

#include <string.h>
#include <string>

#include "noncopyable.h"

namespace util {

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template<int SIZE>
class Buffer : Noncopyable {
public:
    Buffer() : cur_(data_) {}

    ~Buffer() {}

    void append(const char* buf, size_t len) {
        if (static_cast<size_t>(avail()) > len) {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    const char* get_data() const {
        return data_;
    }

    int get_length() const {
        return static_cast<int>(cur_ - data_);
    }

    char* current() {
        return cur_;
    }

    int avail() const {
        return static_cast<int>(end() - cur_);
    }

    void add(size_t len) {
        cur_ += len;
    }

    void reset() {
        cur_ = data_;
    }

    void bzero() {
        memset(data_, 0, sizeof(data_));
    }

    std::string to_string() const {
        return std::string(data_, get_length());
    }

private:
    const char* end() const {
        return data_ + sizeof(data_);
    }

    char data_[SIZE];
    char* cur_;
};

}

#endif