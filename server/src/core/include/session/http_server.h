/**
 * �������֧��http����, HttpServer.h
 * zhangyl 2018.05.16
 */
#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include <list>
#include <memory>
#include <mutex>
#include "event_loop.h"
#include "tcp_server.h"

using namespace network;

// class EventLoop;
// class TcpConnection;
// class TcpServer;
// class EventLoopThreadPool;

class HttpSession;

class HttpServer final {
   public:
    HttpServer()  = default;
    ~HttpServer() = default;

    HttpServer(const HttpServer& rhs)            = delete;
    HttpServer& operator=(const HttpServer& rhs) = delete;

   public:
    bool init(const char* ip, short port, EventLoop* loop);
    void uninit();

    // �����ӵ������û����ӶϿ���������Ҫͨ��conn->connected()���жϣ�һ��ֻ����loop�������
    void onConnected(std::shared_ptr<TcpConnection> conn);
    // ���ӶϿ�
    void onDisconnected(const std::shared_ptr<TcpConnection>& conn);

   private:
    std::unique_ptr<TcpServer>              m_server;
    std::list<std::shared_ptr<HttpSession>> m_sessions;
    std::mutex                              m_sessionMutex; // ���߳�֮�䱣��m_sessions
};

#endif //!__HTTP_SERVER_H__
