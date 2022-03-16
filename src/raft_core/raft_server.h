#ifndef RAFT_SERVER_H_
#define RAFT_SERVER_H_

#include <memory>
#include <stdint.h>
#include <vector>

#include "src/net/tcp_server.h"
#include "src/net/inet_address.h"
#include "src/net/event_loop.h"
#include "src/raft_core/raft_log.h"

namespace xraft {

class RaftServer {
public:
    RaftServer();
    ~RaftServer();

    void init();

    // 运行函数
    void start();
    

private:
    

    // for net
    std::shared_ptr<xraft::net::TcpServer> tcp_server_;
    std::shared_ptr<xraft::net::InetAddress> listen_addr_;
    std::shared_ptr<xraft::net::EventLoop> loop_;

    uint32_t current_term_;
    uint32_t voted_for_;
    std::shared_ptr<xraft::RaftLog> log_;
    uint64_t commit_index_;         // 提交的最大日志下标
    uint64_t last_applied_;         // 引用到状态机的最大日志下标
    std::vector<uint64_t> next_index_;
    std::vector<uint64_t> match_index_;
    std::vector<xraft::net::InetAddress> peers_;

};

}


#endif