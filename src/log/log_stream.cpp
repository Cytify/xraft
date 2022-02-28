#include "src/log/log_stream.h"

#include <algorithm>

namespace xlog {

// 方便负数获取值
const char digits[] = "9876543210123456789";
const char* zero = digits + 9;

const char digits_hex[] = "0123456789ABCDEF";

template<typename T>
size_t convert(char* buf, T value) {
    T i = value;
    char* p = buf;

    do {
        int lsd = static_cast<int>(i % 10);
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

size_t convert_hex(char* buf, uintptr_t value) {
    uintptr_t i = value;
    char* p = buf;

    do {
        int lsd = static_cast<int>(i % 16);
        i /= 16;
        *p++ = digits_hex[lsd];
    } while (i != 0);
    
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

LogStream& LogStream::operator<<(const void* p) {
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if (buffer_.avail() >= kMaxNumericSize) {
        char* buf = buffer_.current();
        buf[0] = '0';
        buf[1] = 'x';
        size_t len = convert_hex(buf + 2, v);
        buffer_.add(len + 2);
    }
    return *this;
}

}