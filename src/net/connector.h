#ifndef CONNECTOR_H_
#define CONNECTOR_H_

#include <functional>
#include <memory>

#include "src/util/noncopyable.h"
#include "src/net/inet_address.h"
#include "src/net/timer_id.h"

namespace xraft {
namespace net {

class Channel;
class EventLoop;

class Connector : util::Noncopyable {
public:
    using NewConnectionCallback = std::function<void(int sockfd)>;

    Connector(EventLoop* loop, const InetAddress& server_addr);

    ~Connector();

    void set_new_connection_callback(const NewConnectionCallback& cb) {
        new_conn_callback_ = cb;
    }

    const InetAddress& get_server_address() const {
        return server_addr_;
    }

    void start();

    void restart();

    void stop();

private:
    enum States {
        Disconnected,
        Connecting,
        Connected,
    };

    static const int kMaxRetryDelayMs = 30 * 1000;
    static const int kInitRetryDelayMs = 500;

    void set_state(States state) {
        state_ = state;
    }

    void start_in_loop();

    void connect();

    void connecting(int sockfd);

    void handle_write();
    
    void handle_error();

    void retry(int sockfd);

    int remove_and_reset_channel();

    void reset_channel();

    EventLoop* loop_;
    InetAddress server_addr_;
    bool connect_;
    States state_;  
    std::shared_ptr<Channel> channel_;
    NewConnectionCallback new_conn_callback_;
    int retry_delay_ms_;
    TimerId timer_id_;
};

using ConnectorPtr = std::shared_ptr<Connector>;

}
}

#endif