#include "epoll_poller.h"

#ifndef WIN32
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

// #ifndef EPOLL_CLOEXEC
// #define EPOLL_CLOEXEC 02000000
// #endif
// #include "../base/Platform.h"
#include "channel.h"
#include "event_loop.h"
#include "log.h"

using namespace network;

namespace {
const int kNew     = -1;
const int kAdded   = 1;
const int kDeleted = 2;
} // namespace

EPollPoller::EPollPoller(EventLoop* loop)
    : epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)), events_(kInitEventListSize), owner_loop_(loop)
{
    if (epoll_fd_ < 0) {
        LOG_FATAL("EPollPoller::EPollPoller");
    }
}

EPollPoller::~EPollPoller()
{
    ::close(epoll_fd_);
}

bool EPollPoller::hasChannel(Channel* channel) const
{
    assertInLoopThread();
    ChannelMap::const_iterator it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}

void EPollPoller::assertInLoopThread() const
{
    owner_loop_->assertInLoopThread();
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    int       numEvents  = ::epoll_wait(epoll_fd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int       savedErrno = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        // LOG_TRACE << numEvents << " events happended";
        fillActiveChannels(numEvents, activeChannels);
        if (static_cast<size_t>(numEvents) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (numEvents == 0) {
        // LOG_TRACE << " nothing happended";
    } else {
        // error happens, log uncommon ones
        if (savedErrno != EINTR) {
            errno = savedErrno;
            LOG_SYSERROR("EPollPoller::poll()");
        }
    }
    return now;
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    for (int i = 0; i < numEvents; ++i) {
        Channel*                   channel = static_cast<Channel*>(events_[static_cast<size_t>(i)].data.ptr);
        int                        fd      = channel->fd();
        ChannelMap::const_iterator it      = channels_.find(fd);
        if (it == channels_.end() || it->second != channel) return;
        channel->set_revents(static_cast<int>(events_[static_cast<size_t>(i)].events));
        activeChannels->push_back(channel);
    }
}

bool EPollPoller::updateChannel(Channel* channel)
{
    assertInLoopThread();
    LOG_DEBUG("fd = %d  events = %d", channel->fd(), channel->events());
    const int index = channel->index();
    if (index == kNew || index == kDeleted) {
        int fd = channel->fd();
        if (index == kNew) {
            if (channels_.find(fd) != channels_.end()) {
                LOG_ERROR("fd = %d  must not exist in channels_", fd);
                return false;
            }

            channels_[fd] = channel;
        } else // index == kDeleted
        {
            if (channels_.find(fd) == channels_.end()) {
                LOG_ERROR("fd = %d  must exist in channels_", fd);
                return false;
            }

            // assert(channels_[fd] == channel);
            if (channels_[fd] != channel) {
                LOG_ERROR("current channel is not matched current fd, fd = %d", fd);
                return false;
            }
        }
        channel->set_index(kAdded);

        return update(EPOLL_CTL_ADD, channel);
    } else {
        // update existing one with EPOLL_CTL_MOD/DEL
        int fd = channel->fd();
        if (channels_.find(fd) == channels_.end() || channels_[fd] != channel || index != kAdded) {
            LOG_ERROR("current channel is not matched current fd, fd = %d, channel = 0x%x", fd, channel);
            return false;
        }

        if (channel->isNoneEvent()) {
            if (update(EPOLL_CTL_DEL, channel)) {
                channel->set_index(kDeleted);
                return true;
            }
            return false;
        } else {
            return update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel* channel)
{
    assertInLoopThread();
    int fd = channel->fd();

    if (channels_.find(fd) == channels_.end() || channels_[fd] != channel || !channel->isNoneEvent()) return;

    int index = channel->index();
    if (index != kAdded && index != kDeleted) return;

    size_t n = channels_.erase(fd);
    if (n != 1) return;

    if (index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

bool EPollPoller::update(int operation, Channel* channel)
{
    struct epoll_event event;
    memset(&event, 0, sizeof event);
    event.events   = static_cast<uint32_t>(channel->events());
    event.data.ptr = channel;
    int fd         = channel->fd();
    if (::epoll_ctl(epoll_fd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_ERROR("epoll_ctl op=%d fd=%d, epollfd=%d, errno=%d, errorInfo: %s", operation, fd, epoll_fd_, errno,
                      strerror(errno));
        } else {
            LOG_ERROR("epoll_ctl op=%d fd=%d, epollfd=%d, errno=%d, errorInfo: %s", operation, fd, epoll_fd_, errno,
                      strerror(errno));
        }

        return false;
    }

    return true;
}

#endif
