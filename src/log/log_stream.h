#ifndef LOG_STREAM_H_
#define LOG_STREAM_H_

#include <string>
#include <string.h>

#include "src/util/buffer.h"
#include "src/util/noncopyable.h"

namespace xlog {

class LogStream : util::Noncopyable {
public:
    LogStream& operator<<(bool v) {
        buffer_.append(v ? "1" : "0", 1);
        return *this;
    }

    LogStream& operator<<(short);

    LogStream& operator<<(unsigned short);

    LogStream& operator<<(int);

    LogStream& operator<<(unsigned int);

    LogStream& operator<<(long);

    LogStream& operator<<(unsigned long);

    LogStream& operator<<(long long);

    LogStream& operator<<(unsigned long long);
    
    LogStream& operator<<(double);
    
    LogStream& operator<<(const void*);

    LogStream& operator<<(float v) {
        return operator<<(static_cast<double>(v));
    }

    LogStream& operator<<(char v) {
        buffer_.append(&v, 1);
        return *this;
    }

    LogStream& operator<<(const char* str) {
        if (str != nullptr) {
            buffer_.append(str, strlen(str));
        } else {
            buffer_.append("null", 4);
        }

        return *this;
    }

    LogStream& operator<<(const unsigned char* str) {
        return operator<<(reinterpret_cast<const char*>(str));
    }

    LogStream& operator<<(const std::string& str) {
        buffer_.append(str.c_str(), str.size());
        return *this;
    }
    
    void append(const char* data, size_t len) { 
        buffer_.append(data, len); 
    }

    const util::Buffer<util::kSmallBuffer>& get_buffer() const {
        return buffer_; 
    }

    void reset_buffer() {
        buffer_.reset();
    }

private:
    // 一个数字字符串的最大长度
    static const int kMaxNumericSize = 48;

    util::Buffer<util::kSmallBuffer> buffer_;

    template<typename T>
    void format_integer(T);
};

}

#endif