#include <unistd.h>

#include "src/log/logger.h"
#include "src/log/log_file.h"

std::unique_ptr<xlog::LogFile> g_log_file;

void output_func(const char* msg, size_t len) {
    g_log_file->append(msg, len);
}

int main(int argc, char* argv[]) {
    char name[256] = { '\0' };
    strncpy(name, argv[0], sizeof(name) - 1);
    g_log_file.reset(new xlog::LogFile(basename(name), 200 * 1000));
    xlog::Logger::set_output_func(output_func);
    std::string line = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

    for (int i = 0; i < 10000; ++i) {
        LOG_INFO << line << i;
        usleep(1000);
    }
}