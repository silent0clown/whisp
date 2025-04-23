#include "channel.h"
#include <sys/epoll.h>
#include <sstream>
// #include "../base/Platform.h"
#include "event_loop.h"
#include "log.h"
#include "poller.h"

using namespace network;

const int Channel::kNoneEvent  = 0;
const int Channel::kReadEvent  = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd__) : loop_(loop), fd_(fd__), events_(0), revents_(0), index_(-1) {}

Channel::~Channel() {}

bool Channel::enableReading()
{
    events_ |= kReadEvent;
    return update();
}

bool Channel::disableReading()
{
    events_ &= ~kReadEvent;

    return update();
}

bool Channel::enableWriting()
{
    events_ |= kWriteEvent;

    return update();
}

bool Channel::disableWriting()
{
    events_ &= ~kWriteEvent;
    return update();
}

bool Channel::disableAll()
{
    events_ = kNoneEvent;
    return update();
}

bool Channel::update()
{
    // addedToLoop_ = true;
    return loop_->updateChannel(this);
}

void Channel::remove()
{
    if (!isNoneEvent()) return;

    loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)
{
    /*
    XPOLLIN ，读事件
    XPOLLPRI，读事件，但表示紧急数据，例如tcp socket的带外数据
    POLLRDNORM , 读事件，表示有普通数据可读　　　
    POLLRDBAND ,　读事件，表示有优先数据可读　　　　
    XPOLLOUT，写事件
    POLLWRNORM , 写事件，表示有普通数据可写
    POLLWRBAND ,　写事件，表示有优先数据可写　　　   　　　　
    XPOLLRDHUP (since Linux 2.6.17)，Stream socket的一端关闭了连接（注意是stream socket，我们知道还有raw socket,dgram
    socket），或者是写端关闭了连接，如果要使用这个事件，必须定义_GNU_SOURCE
    宏。这个事件可以用来判断链路是否发生异常（当然更通用的方法是使用心跳机制）。要使用这个事件，得这样包含头文件：
    　　#define _GNU_SOURCE
      　　#include <poll.h>
    XPOLLERR，仅用于内核设置传出参数revents，表示设备发生错误
    XPOLLHUP，仅用于内核设置传出参数revents，表示设备被挂起，如果poll监听的fd是socket，表示这个socket并没有在网络上建立连接，比如说只调用了socket()函数，但是没有进行connect。
    XPOLLNVAL，仅用于内核设置传出参数revents，表示非法请求文件描述符fd没有打开
    */
    LOG_DEBUG(reventsToString().c_str());
    if ((static_cast<unsigned int>(revents_) & EPOLLHUP) && !(static_cast<unsigned int>(revents_) & EPOLLIN)) {
        if (close_callback_) close_callback_();
    }

    if (revents_ & XPOLLNVAL) {
        LOG_WARN("Channel::handle_event() XPOLLNVAL");
    }

    if (revents_ & (XPOLLERR | XPOLLNVAL)) {
        if (error_callback_) error_callback_();
    }

    if (static_cast<unsigned int>(revents_) & (XPOLLIN | XPOLLPRI | XPOLLRDHUP)) {
        // 当是侦听socket时，readCallback_指向Acceptor::handleRead
        // 当是客户端socket时，调用TcpConnection::handleRead
        if (read_callback_) read_callback_(receiveTime);
    }

    if (revents_ & XPOLLOUT) {
        // 如果是连接状态服的socket，则writeCallback_指向Connector::handleWrite()
        if (write_callback_) write_callback_();
    }
    // eventHandling_ = false;
}

string Channel::reventsToString() const
{
    std::ostringstream oss;
    oss << fd_ << ": ";
    if (revents_ & XPOLLIN) oss << "IN ";
    if (revents_ & XPOLLPRI) oss << "PRI ";
    if (revents_ & XPOLLOUT) oss << "OUT ";
    if (revents_ & XPOLLHUP) oss << "HUP ";
    if (revents_ & XPOLLRDHUP) oss << "RDHUP ";
    if (revents_ & XPOLLERR) oss << "ERR ";
    if (revents_ & XPOLLNVAL) oss << "NVAL ";

    return oss.str().c_str();
}
