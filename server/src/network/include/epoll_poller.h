#pragma once

#ifndef WIN32

#include <map>
#include <vector>

#include "poller.h"
#include "timestamp_util.h"

struct epoll_event;

namespace network {
class EventLoop;

class EPollPoller : public Poller {
   public:
    EPollPoller(EventLoop* loop);
    virtual ~EPollPoller();

    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels);
    virtual bool      updateChannel(Channel* channel);
    virtual void      removeChannel(Channel* channel);

    virtual bool hasChannel(Channel* channel) const;

    void assertInLoopThread() const;

   private:
    static const int kInitEventListSize = 16;

    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
    bool update(int operation, Channel* channel);

   private:
    typedef std::vector<struct epoll_event> EventList;

    int       epoll_fd_;
    EventList events_;

    typedef std::map<int, Channel*> ChannelMap;

    ChannelMap channels_;
    EventLoop* owner_loop_;
};
} // namespace network

#endif
