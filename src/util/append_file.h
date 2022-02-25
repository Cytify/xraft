#ifndef APPEND_FILE_H_
#define APPEND_FILE_H_

#include <string>
#include <sys/types.h>

namespace util {

// 简单封装了一下文件操作
class AppendFile {
public:
    explicit AppendFile(const std::string& filename);

    ~AppendFile();

    void append(const char* buf, size_t len);

    void flush();

    off_t get_written_bytes() const {
        return written_bytes_;
    }

private:
    size_t write(const char* buf, size_t len);

    FILE* fp_;
    char buffer_[64 * 1024];
    off_t written_bytes_;
};

}

#endif