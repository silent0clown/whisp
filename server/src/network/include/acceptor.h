#pragma once

#include <functional>

#include "channel.h"
#include "sockets.h"

namespace network {
class EventLoop;
class InetAddress;

class Acceptor {
   public:
    typedef std::function<void(int sockfd, const InetAddress&)> NewConnectionCallback;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();

    // 设置新连接到来的回调函数
    void setNewConnectionCallback(const NewConnectionCallback& cb)
    {
        new_connection_callback_ = cb;
    }

    bool listenning() const
    {
        return listenning_;
    }
    void listen();

   private:
    void handleRead();

   private:
    EventLoop*            loop_;
    Socket                accept_sockets_;
    Channel               accept_channel_;
    NewConnectionCallback new_connection_callback_;
    bool                  listenning_;

#ifndef WIN32
    int idle_fd_;
#endif
};
} // namespace network
