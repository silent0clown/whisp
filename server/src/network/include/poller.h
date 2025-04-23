#pragma once

// #include <sys/epoll.h>
#include <poll.h>
#include <vector>
#include "timestamp_util.h"

typedef int SOCKET;

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
#define XPOLLIN POLLIN
#define XPOLLPRI POLLPRI
#define XPOLLOUT POLLOUT
#define XPOLLERR POLLERR
#define XPOLLHUP POLLHUP
#define XPOLLNVAL POLLNVAL
#define XPOLLRDHUP POLLRDHUP

#define XEPOLL_CTL_ADD EPOLL_CTL_ADD
#define XEPOLL_CTL_DEL EPOLL_CTL_DEL
#define XEPOLL_CTL_MOD EPOLL_CTL_MOD

namespace network {
class Channel;

class Poller {
   public:
    Poller();
    // ~Poller();
    virtual ~Poller() = default; // ✅ 虚析构函数，确保派生类被正确析构

   public:
    typedef std::vector<Channel*> ChannelList;

    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
    virtual bool      updateChannel(Channel* channel)                  = 0;
    virtual void      removeChannel(Channel* channel)                  = 0;

    virtual bool hasChannel(Channel* channel) const = 0;
};
} // namespace network
