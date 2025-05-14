/**
 * ��ط������࣬MonitorServer.h
 * zhangyl 2018.03.09
 */
#ifndef __MONITOR_SERVER_H__
#define __MONITOR_SERVER_H__
#include <list>
#include <memory>
#include <mutex>
#include "monitor_session.h"
#include "tcp_server.h"

using namespace network;

// class MonitorSession;

class MonitorServer final {
   public:
    MonitorServer()                                    = default;
    ~MonitorServer()                                   = default;
    MonitorServer(const MonitorServer& rhs)            = delete;
    MonitorServer& operator=(const MonitorServer& rhs) = delete;

   public:
    bool init(const char* ip, short port, EventLoop* loop, const char* token);
    void uninit();

    bool isMonitorTokenValid(const char* token);

    // �����ӵ������û����ӶϿ���������Ҫͨ��conn->connected()���жϣ�һ��ֻ����loop�������
    void onConnected(std::shared_ptr<TcpConnection> conn);
    // ���ӶϿ�
    void onDisconnected(const std::shared_ptr<TcpConnection>& conn);

   private:
    std::unique_ptr<TcpServer>                 m_server;
    std::list<std::shared_ptr<MonitorSession>> m_sessions;
    std::mutex                                 m_sessionMutex; // ���߳�֮�䱣��m_sessions
    std::string                                m_token;        // �鿴ĳЩ����������Ҫ��token
};

#endif //!__MONITOR_SERVER_H__