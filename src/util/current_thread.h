#ifndef CURRENT_THREAD_H_
#define CURRENT_THREAD_H_

#include <stdint.h>

namespace util {
namespace current_thread {
/**
 * cache thread id, thread name 
 */
    extern __thread int cached_tid;
    extern __thread char tid_string[32];
    extern __thread int tid_string_length;
    extern __thread const char* thread_name;

    void cache_tid();

    inline int get_tid() {
        if (__builtin_expect(cached_tid == 0, 0)) {
            cache_tid();
        }
        return cached_tid;
    }

    inline const char* get_tid_string() {
        return tid_string;
    }

    inline int get_tid_string_length() {
        return tid_string_length;
    }

    inline const char* get_thread_name() {
        return thread_name;
    }

    bool is_main_thread();
    
    void sleepUsec(int64_t usec);  // for testing

    // string stackTrace(bool demangle);
}
}

#endif
