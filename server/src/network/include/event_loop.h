#pragma once

// #include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

// #include "../base/Platform.h"
#include "callbacks.h"
#include "macros.h"
// #include "sockets.h"
#include "timer_id.h"
#include "timer_queue.h"
#include "timestamp_util.h"

namespace network {
class EventLoop;
class Channel;
class Poller;
class TimerQueue;
class CTimerHeap;

///
/// Reactor, at most one per thread.
///
/// This is an interface class, so don't expose too much details.
class EventLoop {
   public:
    typedef std::function<void()> Functor;

    EventLoop();
    ~EventLoop(); // force out-line dtor, for scoped_ptr members.

    ///
    /// Loops forever.
    ///
    /// Must be called in the same thread as creation of the object.
    ///
    void loop();

    /// Quits loop.
    ///
    /// This is not 100% thread safe, if you call through a raw pointer,
    /// better to call through shared_ptr<EventLoop> for 100% safety.
    void quit();

    ///
    /// Time when poll returns, usually means data arrival.
    ///
    Timestamp pollReturnTime() const
    {
        return poll_return_time_;
    }

    int64_t iteration() const
    {
        return iteration_;
    }

    /// Runs callback immediately in the loop thread.
    /// It wakes up the loop, and run the cb.
    /// If in the same loop thread, cb is run within the function.
    /// Safe to call from other threads.
    void runInLoop(const Functor& cb);
    /// Queues callback in the loop thread.
    /// Runs after finish pooling.
    /// Safe to call from other threads.
    void queueInLoop(const Functor& cb);

    // timers，时间单位均是微秒
    ///
    /// Runs callback at 'time'.
    /// Safe to call from other threads.
    ///
    TimerId runAt(const Timestamp& time, const TimerCallback& cb);
    ///
    /// Runs callback after @c delay seconds.
    /// Safe to call from other threads.
    ///
    TimerId runAfter(int64_t delay, const TimerCallback& cb);
    ///
    /// Runs callback every @c interval seconds.
    /// Safe to call from other threads.
    ///
    TimerId runEvery(int64_t interval, const TimerCallback& cb);
    ///
    /// Cancels the timer.
    /// Safe to call from other threads.
    ///
    void cancel(TimerId timerId, bool off);

    void remove(TimerId timerId);

    TimerId runAt(const Timestamp& time, TimerCallback&& cb);
    TimerId runAfter(int64_t delay, TimerCallback&& cb);
    TimerId runEvery(int64_t interval, TimerCallback&& cb);

    void setFrameFunctor(const Functor& cb);

    // internal usage
    bool updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    // pid_t threadId() const { return threadId_; }
    void assertInLoopThread()
    {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }
    bool isInLoopThread() const
    {
        return thread_id_ == std::this_thread::get_id();
    }
    bool eventHandling() const
    {
        return event_handling_;
    }

    const std::thread::id getThreadID() const
    {
        return thread_id_;
    }

   private:
    bool createWakeupfd();
    bool wakeup();
    void abortNotInLoopThread();
    bool handleRead(); // waked up handler
    void doOtherTasks();

    void printActiveChannels() const; // DEBUG

   private:
    typedef std::vector<Channel*> ChannelList;

    bool                        looping_;
    bool                        quit_;
    bool                        event_handling_;
    bool                        doing_other_tasks_;
    const std::thread::id       thread_id_;
    Timestamp                   poll_return_time_;
    std::unique_ptr<Poller>     poller_;
    std::unique_ptr<TimerQueue> timer_queue_;
    int64_t                     iteration_;
#ifdef WIN32
    SOCKET wakeup_fd_Send;
    SOCKET wakeup_fd_Listen;
    SOCKET wakeup_fd_Recv;

    // int                               fdpipe_[2];
#else
    SOCKET wakeup_fd_; // TODO: 这个fd什么时候释放？
#endif
    // unlike in TimerQueue, which is an internal class,
    // we don't expose Channel to client.
    std::unique_ptr<Channel> wakeup_channel_;

    // scratch variables
    ChannelList active_channels_;
    Channel*    current_active_channel_;

    std::mutex           mutex_;
    std::vector<Functor> pending_functors_; // Guarded by mutex_

    Functor frame_functor_;
};

} // namespace network
