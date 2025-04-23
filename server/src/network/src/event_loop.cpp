#include "event_loop.h"

#include <string.h>
#include <sys/eventfd.h> // for eventfd
#include <sstream>       // for sstream

#include "channel.h"
#include "epoll_poller.h"
#include "log.h"
#include "sockets.h"

using namespace network;

// 内部侦听唤醒fd的侦听端口，因此外部可以再使用这个端口
// #define INNER_WAKEUP_LISTEN_PORT 10000

thread_local EventLoop* t_loopInThisThread = 0;

const int kPollTimeMs = 1;

EventLoop* getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

// 在线程函数中创建eventloop
EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      event_handling_(false),
      doing_other_tasks_(false),
      thread_id_(std::this_thread::get_id()),
      timer_queue_(new TimerQueue(this)),
      iteration_(0L),
      current_active_channel_(NULL)
{
    createWakeupfd();

#ifdef WIN32
    wakeup_channel_.reset(new Channel(this, wakeup_fd_Recv));
    poller_.reset(new SelectPoller(this));

#else
    wakeup_channel_.reset(new Channel(this, wakeup_fd_));
    poller_.reset(new EPollPoller(this));
#endif

    if (t_loopInThisThread) {
        LOG_FATAL("Another EventLoop  exists in this thread ");
    } else {
        t_loopInThisThread = this;
    }
    wakeup_channel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // we are always reading the wakeupfd
    wakeup_channel_->enableReading();
}

EventLoop::~EventLoop()
{
    assertInLoopThread();
    LOG_DEBUG("EventLoop 0x%x destructs.", this);

    // std::stringstream ss;
    // ss << "eventloop destructs threadid = " << threadId_;
    // std::cout << ss.str() << std::endl;

    wakeup_channel_->disableAll();
    wakeup_channel_->remove();

#ifdef WIN32
    sockets::close(wakeup_fd_Send);
    sockets::close(wakeup_fd_Recv);
    sockets::close(wakeup_fd_Listen);
#else
    sockets::close(wakeup_fd_);
#endif

    //_close(fdpipe_[0]);
    //_close(fdpipe_[1]);

    t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
    // assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_    = false; // FIXME: what if someone calls quit() before loop() ?
    LOG_DEBUG("EventLoop 0x%x  start looping", this);

    while (!quit_) {
        timer_queue_->doTimer();

        active_channels_.clear();
        poll_return_time_ = poller_->poll(kPollTimeMs, &active_channels_);
        // if (Logger::logLevel() <= Logger::TRACE)
        //{
        printActiveChannels();
        //}
        ++iteration_;
        // TODO sort channel by priority
        event_handling_ = true;
        for (const auto& it : active_channels_) {
            current_active_channel_ = it;
            current_active_channel_->handleEvent(poll_return_time_);
        }
        current_active_channel_ = nullptr;
        event_handling_         = false;
        doOtherTasks();

        if (frame_functor_) {
            frame_functor_();
        }
    }

    LOG_DEBUG("EventLoop 0x%0x stop looping", this);
    looping_ = false;

    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string stid = oss.str();
    LOG_INFO("Exiting loop, EventLoop object: 0x%x , threadID: %s", this, stid.c_str());
}

void EventLoop::quit()
{
    quit_ = true;
    // There is a chance that loop() just executes while(!quit_) and exists,
    // then EventLoop destructs, then we are accessing an invalid object.
    // Can be fixed using mutex_ in both places.
    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(const Functor& cb)
{
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Functor& cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pending_functors_.push_back(cb);
    }

    if (!isInLoopThread() || doing_other_tasks_) {
        wakeup();
    }
}

void EventLoop::setFrameFunctor(const Functor& cb)
{
    frame_functor_ = cb;
}

TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb)
{
    // 只执行一次
    return timer_queue_->addTimer(cb, time, 0, 1);
}

TimerId EventLoop::runAfter(int64_t delay, const TimerCallback& cb)
{
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, cb);
}

TimerId EventLoop::runEvery(int64_t interval, const TimerCallback& cb)
{
    Timestamp time(addTime(Timestamp::now(), interval));
    //-1表示一直重复下去
    return timer_queue_->addTimer(cb, time, interval, -1);
}

TimerId EventLoop::runAt(const Timestamp& time, TimerCallback&& cb)
{
    return timer_queue_->addTimer(std::move(cb), time, 0, 1);
}

TimerId EventLoop::runAfter(int64_t delay, TimerCallback&& cb)
{
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(int64_t interval, TimerCallback&& cb)
{
    Timestamp time(addTime(Timestamp::now(), interval));
    return timer_queue_->addTimer(std::move(cb), time, interval, -1);
}

void EventLoop::cancel(TimerId timerId, bool off)
{
    return timer_queue_->cancel(timerId, off);
}

void EventLoop::remove(TimerId timerId)
{
    return timer_queue_->removeTimer(timerId);
}

bool EventLoop::updateChannel(Channel* channel)
{
    // assert(channel->ownerLoop() == this);
    if (channel->ownerLoop() != this) return false;

    assertInLoopThread();

    return poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    // assert(channel->ownerLoop() == this);
    if (channel->ownerLoop() != this) return;

    assertInLoopThread();
    if (event_handling_) {
        // assert(current_active_channel_ == channel || std::find(activeChannels_.begin(), activeChannels_.end(),
        // channel)
        // == activeChannels_.end());
    }

    LOG_DEBUG("Remove channel, channel = 0x%x, fd = %d", channel, channel->fd());
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
    // assert(channel->ownerLoop() == this);
    assertInLoopThread();
    return poller_->hasChannel(channel);
}

bool EventLoop::createWakeupfd()
{
#ifdef WIN32
    // if (_pipe(fdpipe_, 256, O_BINARY) == -1)
    //{
    //     //让程序挂掉
    //     LOG_FATAL("Unable to create pipe, EventLoop: 0x%x", this);
    //     return false;
    // }

    wakeup_fd_Listen = sockets::createOrDie();
    wakeup_fd_Send   = sockets::createOrDie();

    // Windows上需要创建一对socket
    struct sockaddr_in bindaddr;
    bindaddr.sin_family      = AF_INET;
    bindaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    // 将port设为0，然后进行bind，再接着通过getsockname来获取port，这可以满足获取随机端口的情况。
    bindaddr.sin_port = 0;
    sockets::setReuseAddr(wakeup_fd_Listen, true);
    sockets::bindOrDie(wakeup_fd_Listen, bindaddr);
    sockets::listenOrDie(wakeup_fd_Listen);

    struct sockaddr_in serveraddr;
    int                serveraddrlen = sizeof(serveraddr);
    if (getsockname(wakeup_fd_Listen, (sockaddr*) &serveraddr, &serveraddrlen) < 0) {
        // 让程序挂掉
        LOG_FATAL("Unable to bind address info, EventLoop: 0x%x", this);
        return false;
    }

    int useport = ntohs(serveraddr.sin_port);
    LOG_DEBUG("wakeup fd use port: %d", useport);

    // serveraddr.sin_family = AF_INET;
    // serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // serveraddr.sin_port = htons(INNER_WAKEUP_LISTEN_PORT);
    if (::connect(wakeup_fd_Send, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) < 0) {
        // 让程序挂掉
        LOG_FATAL("Unable to connect to wakeup peer, EventLoop: 0x%x", this);
        return false;
    }

    struct sockaddr_in clientaddr;
    socklen_t          clientaddrlen = sizeof(clientaddr);
    wakeup_fd_Recv                   = ::accept(wakeup_fd_Listen, (struct sockaddr*) &clientaddr, &clientaddrlen);
    if (wakeup_fd_Recv < 0) {
        // 让程序挂掉
        LOG_FATAL("Unable to accept wakeup peer, EventLoop: 0x%x", this);
        return false;
    }

    sockets::setNonBlockAndCloseOnExec(wakeup_fd_Send);
    sockets::setNonBlockAndCloseOnExec(wakeup_fd_Recv);

#else
    // Linux上一个eventfd就够了，可以实现读写
    wakeup_fd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (wakeup_fd_ < 0) {
        // 让程序挂掉
        LOG_FATAL("Unable to create wakeup eventfd, EventLoop: 0x%x", this);
        return false;
    }

#endif

    return true;
}

void EventLoop::abortNotInLoopThread()
{
    std::stringstream ss;
    ss << "threadid_ = " << thread_id_ << " this_thread::get_id() = " << std::this_thread::get_id();
    LOG_FATAL("EventLoop::abortNotInLoopThread - EventLoop %s", ss.str().c_str());
}

bool EventLoop::wakeup()
{
    uint64_t one = 1;
#ifdef WIN32
    int32_t n = sockets::write(wakeup_fd_Send, &one, sizeof(one));
#else
    int32_t n = sockets::write(wakeup_fd_, &one, sizeof(one));
#endif

    if (n != sizeof one) {
#ifdef WIN32
        DWORD error = WSAGetLastError();
        LOG_SYSERROR("EventLoop::wakeup() writes %d  bytes instead of 8, fd: %d, error: %d", n, wakeup_fd_Send,
                     (int32_t) error);
#else
        int error = errno;
        LOG_SYSERROR("EventLoop::wakeup() writes %d  bytes instead of 8, fd: %d, error: %d, errorinfo: %s", n,
                     wakeup_fd_, error, strerror(error));
#endif

        return false;
    }

    return true;
}

bool EventLoop::handleRead()
{
    uint64_t one = 1;
#ifdef WIN32
    int32_t n = sockets::read(wakeup_fd_Recv, &one, sizeof(one));
#else
    int32_t n = sockets::read(wakeup_fd_, &one, sizeof(one));
#endif

    if (n != sizeof one) {
#ifdef WIN32
        DWORD error = WSAGetLastError();
        LOG_SYSERROR("EventLoop::wakeup() read %d  bytes instead of 8, fd: %d, error: %d", n, wakeup_fd_Recv,
                     (int32_t) error);
#else
        int error = errno;
        LOG_SYSERROR("EventLoop::wakeup() read %d  bytes instead of 8, fd: %d, error: %d, errorinfo: %s", n, wakeup_fd_,
                     error, strerror(error));
#endif
        return false;
    }

    return true;
}

void EventLoop::doOtherTasks()
{
    std::vector<Functor> functors;
    doing_other_tasks_ = true;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pending_functors_);
    }

    for (size_t i = 0; i < functors.size(); ++i) {
        functors[i]();
    }

    doing_other_tasks_ = false;
}

void EventLoop::printActiveChannels() const
{
    // TODO: 改成for-each 语法
    // std::vector<Channel*>
    for (const auto& iter : active_channels_) {
        LOG_DEBUG("{%s}", iter->reventsToString().c_str());
    }
}
