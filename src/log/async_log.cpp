#include "async_log.h"

#include "log_file.h"
#include "src/util/timestamp.h"

namespace xlog {

AsyncLog::AsyncLog(const std::string& basename, off_t roll_size, int flush_interval)
    : flush_interval_(flush_interval), basename_(basename), roll_size_(roll_size),
      running_(false), thread_(), mutex_(), cond_(), latch_(1),
      current_buffer_(new Buffer), next_buffer_(new Buffer), buffers() {
    current_buffer_->bzero();
    next_buffer_->bzero();
    buffers.reserve(16);
}

AsyncLog::~AsyncLog() {
    if (running_) {
        stop();
    }
}

void AsyncLog::start() {
    running_ = true;
    thread_ = std::thread(&AsyncLog::thread_func, this);
    latch_.wait();
}

void AsyncLog::stop() {
    running_ = false;
    cond_.notify_one();
    thread_.join();
}

void AsyncLog::append(const char* buf, size_t len) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (static_cast<size_t>(current_buffer_->avail()) > len) {
        current_buffer_->append(buf, len);
    } else {
        buffers.push_back(std::move(current_buffer_));

        if (next_buffer_) {
            current_buffer_ = std::move(next_buffer_);
        } else {
            current_buffer_.reset(new Buffer);
        }
        current_buffer_->append(buf, len);
        cond_.notify_one();             // 写满了一块缓存
    }
}

void AsyncLog::thread_func() {
    latch_.count_down();
    LogFile log_file(basename_, roll_size_);
    BufferPtr new_buffer_1(new Buffer);
    new_buffer_1->bzero();
    BufferPtr new_buffer_2(new Buffer);
    new_buffer_2->bzero();
    BufferVector buffers_to_write;
    buffers_to_write.reserve(16);
    while (running_) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (buffers.empty()) {
                cond_.wait_for(lock, std::chrono::seconds(flush_interval_));
            }
            buffers.push_back(std::move(current_buffer_));
            current_buffer_ = std::move(new_buffer_1);
            buffers_to_write.swap(buffers);
            if (!next_buffer_) {
                next_buffer_ = std::move(new_buffer_2);
            }
        }

        // 日志太多了，只保留前两个buffer
        if (buffers_to_write.size() > 2) {
            char buf[256];      
            snprintf(buf, sizeof(buf), "Dropped log messages at %s, %zd larger buffers\n",
                     util::Timestamp::now().to_format_string().c_str(), buffers_to_write.size() - 2);
            fputs(buf, stderr);
            log_file.append(buf, strlen(buf));
            buffers_to_write.erase(buffers_to_write.begin() + 2, buffers_to_write.end());
        }

        // 写入日志
        for (const auto& buffer : buffers_to_write) {
            log_file.append(buffer->get_data(), buffer->get_length());
        }

        // 重新设置new_buffer_1和new_buffer_2，循环利用
        if (buffers_to_write.size() > 2) {
            buffers_to_write.resize(2);
        }

        if (!new_buffer_1) {
            new_buffer_1 = std::move(buffers_to_write.back());
            buffers_to_write.pop_back();
            new_buffer_1->reset();
        }

        if (!new_buffer_2) {
            new_buffer_2 = std::move(buffers_to_write.back());
            buffers_to_write.pop_back();
            new_buffer_2->reset();
        }
        buffers_to_write.clear();
        log_file.flush();
    }
    log_file.flush();
}

}