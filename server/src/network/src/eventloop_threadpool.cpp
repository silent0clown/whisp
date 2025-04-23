#include "eventloop_threadpool.h"
#include <assert.h>
#include <stdio.h>
#include <sstream>
#include <string>
#include "event_loop.h"
#include "eventloop_thread.h"

using namespace network;

EventLoopThreadPool::EventLoopThreadPool() : base_loop_(NULL), started_(false), num_threads_(0), next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool()
{
    // Don't delete loop, it's stack variable
}

void EventLoopThreadPool::init(EventLoop* baseLoop, int numThreads)
{
    num_threads_ = numThreads;
    base_loop_   = baseLoop;
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
    // assert(baseLoop_);
    if (base_loop_ == NULL) return;

    // assert(!started_);
    if (started_) return;

    base_loop_->assertInLoopThread();

    started_ = true;

    for (int i = 0; i < num_threads_; ++i) {
        char buf[128];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);

        std::unique_ptr<EventLoopThread> t(new EventLoopThread(cb, buf));
        // EventLoopThread* t = new EventLoopThread(cb, buf);
        loops_.push_back(t->startLoop());
        threads_.push_back(std::move(t));
    }
    if (num_threads_ == 0 && cb) {
        cb(base_loop_);
    }
}

void EventLoopThreadPool::stop()
{
    for (auto& iter : threads_) {
        iter->stopLoop();
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    base_loop_->assertInLoopThread();
    // assert(started_);
    if (!started_) return NULL;

    EventLoop* loop = base_loop_;

    if (!loops_.empty()) {
        // round-robin
        loop = loops_[static_cast<size_t>(next_)];
        ++next_;
        if (size_t(next_) >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop;
}

EventLoop* EventLoopThreadPool::getLoopForHash(size_t hashCode)
{
    base_loop_->assertInLoopThread();
    EventLoop* loop = base_loop_;

    if (!loops_.empty()) {
        loop = loops_[hashCode % loops_.size()];
    }
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
    base_loop_->assertInLoopThread();
    if (loops_.empty()) {
        return std::vector<EventLoop*>(1, base_loop_);
    } else {
        return loops_;
    }
}

const std::string EventLoopThreadPool::info() const
{
    std::stringstream ss;
    ss << "print threads id info " << endl;
    for (size_t i = 0; i < loops_.size(); i++) {
        ss << i << ": id = " << loops_[i]->getThreadID() << endl;
    }
    return ss.str();
}
