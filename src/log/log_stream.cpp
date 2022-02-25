#include "log_stream.h"

#include <algorithm>

namespace xlog {

// 方便负数获取值
const char digits[] = "9876543210123456789";
const char* zero = digits + 9;

template<typename T>
size_t convert(char* buf, T value) {
    T i = value;
    char* p = buf;

    do {
        int lsd = i % 10;
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);

    if (value < 0) {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

template<typename T>
void LogStream::format_integer(T v) {
    // 可用空间不足，丢弃
    if (buffer_.avail() >= kMaxNumericSize) {
        size_t len = convert(buffer_.current(), v);
        buffer_.add(len);
    }
}

LogStream& LogStream::operator<<(short v) {
    return operator<<(static_cast<int>(v));
}

LogStream& LogStream::operator<<(unsigned short v) {
    return operator<<(static_cast<int>(v));
}

LogStream& LogStream::operator<<(int v) {
    format_integer(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
    format_integer(v);
    return *this;
}

LogStream& LogStream::operator<<(long v) {
    format_integer(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long v) {
    format_integer(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v) {
    format_integer(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) {
    format_integer(v);
    return *this;
}

LogStream& LogStream::operator<<(double v) {
    if (buffer_.avail() > kMaxNumericSize) {
        size_t len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
        buffer_.add(len);
    }
    return *this;
}

}