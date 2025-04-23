#pragma once

#include <functional>
#include <memory>
#include "inet_address.h"

namespace network {

class Channel;
class EventLoop;

class Connector : public std::enable_shared_from_this<Connector> {
   public:
    typedef std::function<void(int sockfd)> NewConnectionCallback;

    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    {
        new_connection_callback_ = cb;
    }

    void start();
    void restart();
    void stop();

    const InetAddress& serverAddress() const
    {
        return server_addr_;
    }

   private:
    enum State { kDisconnected, kConnecting, kConnected };
    static const int kMaxRetryDelayMs  = 30 * 1000;
    static const int kInitRetryDelayMs = 500;

    void setState(State s)
    {
        state_ = s;
    }
    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    int  removeAndResetChannel();
    void resetChannel();

   private:
    EventLoop*               loop_;
    InetAddress              server_addr_;
    bool                     connect_;
    State                    state_;
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback    new_connection_callback_;
    int                      retry_delay_ms_;
};
} // namespace network
