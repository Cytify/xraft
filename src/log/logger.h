#ifndef LOGGER_H_
#define LOGGER_H_

#include "src/log/log_stream.h"
#include "src/util/noncopyable.h"
#include "src/util/timestamp.h"

namespace xlog {

class SourceFile {
public:
    template<int N>
    SourceFile(const char (&arr)[N]) : data_(arr), size_(N - 1) {
        const char* slash = strrchr(data_, '/');
        if (slash) {
            data_ = slash + 1;
            size_ -= static_cast<int>(data_ - arr);
        }
    }

    explicit SourceFile(const char* filename) : data_(filename) {
        const char* slash = strrchr(filename, '/');
        if (slash) {
            data_ = slash + 1;
        }
        size_ = static_cast<int>(strlen(data_));
    }

    const char* get_data() const {
        return data_;
    }

    int get_size() const {
        return size_;
    }

private:
    const char* data_;
    int size_;
};

class Logger : util::Noncopyable {
public:
    enum LogLevel {
        INFO,
        WARN,
        ERROR,
        NUM_LOG_LEVELS,
    };

    typedef void (*OutputFunc)(const char* msg, size_t len);
    typedef void (*FlushFunc)();

    Logger(SourceFile file, int line, LogLevel level);

    ~Logger();

    LogStream& stream() {
        return impl_.stream_;
    } 

    static void set_output_func(OutputFunc func);

private:
    class Impl {
    public:
        Impl(const SourceFile& file, int line, LogLevel level);

        void format_time();

        void finish();
        
        util::Timestamp time_;
        LogStream stream_;
        LogLevel level_;
        SourceFile basename_;
        int line_;
    };

    Impl impl_;
};

#define LOG_INFO xlog::Logger(__FILE__, __LINE__, xlog::Logger::INFO).stream()

#define LOG_WARN xlog::Logger(__FILE__, __LINE__, xlog::Logger::WARN).stream()

#define LOG_ERROR xlog::Logger(__FILE__, __LINE__, xlog::Logger::ERROR).stream()

}

#endif