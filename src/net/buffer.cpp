#include "src/net/buffer.h"

#include <sys/uio.h>

namespace xraft {
namespace net {

ssize_t Buffer::read_fd(int fd) {
    char extra_buf[65536];
    struct iovec vec[2];
    const size_t writable = writable_bytes();
    vec[0].iov_base = begin() + writer_index_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extra_buf;
    vec[1].iov_len = sizeof(extra_buf);

    const ssize_t n = readv(fd, vec, 2);
    if (n < 0) {
        // errno
    } else if (static_cast<size_t>(n) <= writable) {
        writer_index_ += n;
    } else {
        writer_index_ = buffer_.size();
        append(extra_buf, n - writable);
    }

    return n;
}

}
}