#include "append_file.h"

#include <stdio.h>

namespace util {

AppendFile::AppendFile(const std::string& filename) 
    : fp_(fopen(filename.c_str(), "ae")), written_bytes_(0) {   // e 为 O_CLOEXEC
    if (fp_) {
        // 设置缓冲区，这一块写满后，才会全部刷新到磁盘上，或者手动调用fflush刷新
        setbuffer(fp_, buffer_, sizeof(buffer_));
    }
}

AppendFile::~AppendFile() {
    fclose(fp_);
}

void AppendFile::append(const char* buf, size_t len) {
    size_t written = 0;

    while (written != len) {
        size_t remain = len - written;
        size_t n = this->write(buf, remain);
        if (n != remain) {
            int err = ferror(fp_);
            if (err) {
                fprintf(stderr, "AppendFile::append() failed %d\n",err);
                break;
            }
        }
        written += n;
    }

    written_bytes_ += written;
}

void AppendFile::flush() {
    fflush(fp_);
}

size_t AppendFile::write(const char* buf, size_t len) {
    // 调用无所版本，因为无锁版本速度快些，且此处同时仅有1个线程写1个文件
    return fwrite_unlocked(buf, 1, len, fp_);
}

}