#pragma once

#include <functional>
// #include <memory>

#include "timestamp_util.h"

namespace network {
class EventLoop;

class Channel {
   public:
    typedef std::function<void()>          EventCallback;
    typedef std::function<void(Timestamp)> ReadEventCallback;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handleEvent(Timestamp receiveTime);
    void setReadCallback(const ReadEventCallback& cb)
    {
        read_callback_ = cb;
    }
    void setWriteCallback(const EventCallback& cb)
    {
        write_callback_ = cb;
    }
    void setCloseCallback(const EventCallback& cb)
    {
        close_callback_ = cb;
    }
    void setErrorCallback(const EventCallback& cb)
    {
        error_callback_ = cb;
    }

    int fd() const
    {
        return fd_;
    }
    int events() const
    {
        return events_;
    }
    void set_revents(int revt)
    {
        revents_ = revt;
    }
    void add_revents(int revt)
    {
        revents_ |= revt;
    }
    // int revents() const { return revents_; }
    bool isNoneEvent() const
    {
        return events_ == kNoneEvent;
    }

    bool enableReading();
    bool disableReading();
    bool enableWriting();
    bool disableWriting();
    bool disableAll();

    bool isWriting() const
    {
        return events_ & kWriteEvent;
    }

    int index()
    {
        return index_;
    }
    void set_index(int idx)
    {
        index_ = idx;
    }

    string reventsToString() const;

    EventLoop* ownerLoop()
    {
        return loop_;
    }
    void remove();

   private:
    bool update();

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    const int  fd_;
    int        events_;
    int        revents_;
    int        index_;

    ReadEventCallback read_callback_;
    EventCallback     write_callback_;
    EventCallback     close_callback_;
    EventCallback     error_callback_;
};
} // namespace network
