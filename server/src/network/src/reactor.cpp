#include "reactor.h"
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
// #include <iostream>

Channel::Channel(int fd, EventLoop* loop) : fd_(fd), events_(0), loop_(loop) {}

void Channel::handleEvent()
{
    if (readCallback_) readCallback_();
    if (writeCallback_) writeCallback_();
}

void Channel::enableReading()
{
    events_ |= EPOLLIN;
    loop_->updateChannel(this);
}

void Channel::disableReading()
{
    events_ &= ~EPOLLIN;
    loop_->updateChannel(this);
}

void Channel::enableWriting()
{
    events_ |= EPOLLOUT;
    loop_->updateChannel(this);
}

void Channel::disableWriting()
{
    events_ &= ~EPOLLOUT;
    loop_->updateChannel(this);
}

// EventLoop

EventLoop::EventLoop()
{
    epollFd_ = epoll_create1(0);
    if (epollFd_ < 0) {
        perror("epoll_create1");
        exit(1);
    }
}

EventLoop::~EventLoop()
{
    close(epollFd_);
}

void EventLoop::addChannel(Channel* channel)
{
    struct epoll_event ev;
    ev.events   = channel->events();
    ev.data.ptr = channel;
    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, channel->fd(), &ev) < 0) {
        perror("epoll_ctl ADD");
    }
    channels_[channel->fd()] = channel;
}

void EventLoop::updateChannel(Channel* channel)
{
    struct epoll_event ev;
    ev.events   = channel->events();
    ev.data.ptr = channel;
    if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, channel->fd(), &ev) < 0) {
        perror("epoll_ctl MOD");
    }
}

void EventLoop::loop()
{
    struct epoll_event events[64];
    while (true) {
        int n = epoll_wait(epollFd_, events, 64, -1);
        for (int i = 0; i < n; ++i) {
            Channel* ch = static_cast<Channel*>(events[i].data.ptr);
            ch->handleEvent();
        }
    }
}
