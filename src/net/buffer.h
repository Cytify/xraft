#ifndef BUFFER_H_
#define BUFFER_H_

#include <vector>
#include <algorithm>
#include <string>
#include <assert.h>

#include "src/util/noncopyable.h"

namespace xraft {
namespace net {

class Buffer : util::Noncopyable {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    Buffer() : buffer_(kCheapPrepend + kInitialSize), 
        reader_index_(kCheapPrepend), writer_index_(kCheapPrepend) {}

    void swap(Buffer& rhs) {
        buffer_.swap(rhs.buffer_);
        std::swap(reader_index_, rhs.reader_index_);
        std::swap(writer_index_, rhs.writer_index_);
    }

    size_t readable_bytes() const {
        return writer_index_ - reader_index_;
    }

    size_t writable_bytes() const {
        return buffer_.size() - writer_index_;
    }

    size_t prependable_bytes() const {
        return reader_index_;
    }
    
    const char* peek() const {
        return begin() + reader_index_;
    }

    void retrieve(size_t len) {
        assert(len <= readable_bytes());
        reader_index_ += len;
    }

    void retrieve_until(const char* end) {
        assert(peek() <= end);
        assert(end <= begin_write());
        retrieve(end - peek());
    }

    void retrieve_all() {
        reader_index_ = kCheapPrepend;
        writer_index_ = kCheapPrepend;
    }

    std::string retrieve_as_string() {
        std::string str(peek(), readable_bytes());
        retrieve_all();
        return str;
    }

    void append(const std::string& str) {
        append(str.c_str(), str.size());
    }

    void append(const char* data, size_t len) {
        ensure_writable_bytes(len);
        std::copy(data, data + len, begin_write());
        has_written(len);
    }

    void append(const void* data, size_t len) {
        append(static_cast<const char*>(data), len);
    }

    void ensure_writable_bytes(size_t len) {
        if (writable_bytes() < len) {
            make_sapce(len);
        }
        assert(writable_bytes() >= len);
    }

    char* begin_write() {
        return begin() + writer_index_;
    }

    const char* begin_write() const {
        return begin() + writer_index_;
    }

    void has_written(size_t len) {
        writer_index_ += len;
    }

    void prepend(const void* data, size_t len) {
        assert(len <= prependable_bytes());
        reader_index_ -= len;
        const char* d = static_cast<const char*>(data);
        std::copy(d, d + len, begin() + reader_index_);
    }

    void shrink(size_t reserve) {
        std::vector<char> buf(kCheapPrepend + readable_bytes() + reserve);
        std::copy(peek(), peek() + readable_bytes(), buf.begin() + kCheapPrepend);
        buf.swap(buffer_);
    }

    // 将数据读到buffer里，内部调用了readv
    ssize_t read_fd(int fd);

private:
    char* begin() {
        return &*buffer_.begin();
    }

    const char* begin() const {
        return &*buffer_.begin(); 
    }

    void make_sapce(size_t len) {
        // 原有的空闲空间不够用
        if (writable_bytes() + prependable_bytes() < len + kCheapPrepend) {
            buffer_.resize(writer_index_ + len);
        } else {
            assert(kCheapPrepend < reader_index_);
            size_t readable = readable_bytes();
            std::copy(begin() + reader_index_, begin() + writer_index_, begin() + kCheapPrepend);
            reader_index_ = kCheapPrepend;
            writer_index_ = kCheapPrepend + readable;
            assert(readable == readable_bytes());
        }
    }

    std::vector<char> buffer_;
    size_t reader_index_;
    size_t writer_index_;
};

}
}

#endif