#pragma once

#include <functional>
#include <unordered_map>
#include <sys/epoll.h>

class EventLoop;

class Channel {
public:
    using EventCallback = std::function<void()>;

    Channel(int fd, EventLoop* loop);
    void handleEvent();

    void setReadCallback(EventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }

    void enableReading();
    void disableReading();
    void enableWriting();
    void disableWriting();

    int fd() const { return fd_; }
    uint32_t events() const { return events_; }

private:
    int fd_;
    uint32_t events_;
    EventLoop* loop_;
    EventCallback readCallback_;
    EventCallback writeCallback_;
};

class EventLoop {
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void addChannel(Channel* channel);
    void updateChannel(Channel* channel);

private:
    int epollFd_;
    std::unordered_map<int, Channel*> channels_;
};
