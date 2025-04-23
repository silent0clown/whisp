#pragma once

#include <functional>
#include <memory>
#include "timestamp_util.h"

namespace network {
class ByteBuffer;
class TcpConnection;
typedef std::shared_ptr<TcpConnection>                       TcpConnectionPtr;
typedef std::function<void()>                                TimerCallback;
typedef std::function<void(const TcpConnectionPtr&)>         ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr&)>         CloseCallback;
typedef std::function<void(const TcpConnectionPtr&)>         WriteCompleteCallback;
typedef std::function<void(const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;

typedef std::function<void(const TcpConnectionPtr&, ByteBuffer*, Timestamp)> MessageCallback;

void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn, ByteBuffer* buffer, Timestamp receiveTime);
} // namespace network
