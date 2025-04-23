#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

namespace network {
class EventLoop;

class EventLoopThread {
   public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;

    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(), const std::string& name = "");
    ~EventLoopThread();
    EventLoop* startLoop();
    void       stopLoop();

   private:
    void threadFunc();

    EventLoop*                   loop_;
    bool                         exiting_;
    std::unique_ptr<std::thread> thread_;
    std::mutex                   mutex_;
    std::condition_variable      cond_;
    ThreadInitCallback           callback_;
};

} // namespace network
