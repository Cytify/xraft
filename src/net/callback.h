#ifndef CALLBACK_H_
#define CALLBACK_H_

#include <functional>
#include <memory>

#include "src/util/timestamp.h"

namespace xraft {
namespace net {

class Buffer;

class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using TimerCallback = std::function<void()>;

using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;

using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer* buffer, util::Timestamp)>;

using CloseCallback = std::function<void(const TcpConnectionPtr&)>;

using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;

}
}

#endif