/**
 *  �������������࣬IMServer.h
 *  zhangyl 2017.03.09
 **/
#pragma once
#include <atomic>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include "chat_session.h"
#include "event_loop.h"
#include "tcp_server.h"

using namespace network;

enum CLIENT_TYPE { CLIENT_TYPE_UNKOWN, CLIENT_TYPE_PC, CLIENT_TYPE_ANDROID, CLIENT_TYPE_IOS, CLIENT_TYPE_MAC };

struct StoredUserInfo {
    int32_t     userid;
    std::string username;
    std::string password;
    std::string nickname;
};

class ChatServer final {
   public:
    ChatServer();
    ~ChatServer() = default;

    ChatServer(const ChatServer& rhs)            = delete;
    ChatServer& operator=(const ChatServer& rhs) = delete;

    bool init(const char* ip, short port, EventLoop* loop);
    void uninit();

    void enableLogPackageBinary(bool enable);
    bool isLogPackageBinaryEnabled();

    void getSessions(std::list<std::shared_ptr<ChatSession>>& sessions);
    // �û�id��clienttype��Ψһȷ��һ��session
    bool getSessionByUserIdAndClientType(std::shared_ptr<ChatSession>& session, int32_t userid, int32_t clientType);

    bool getSessionsByUserId(std::list<std::shared_ptr<ChatSession>>& sessions, int32_t userid);

    // ��ȡ�û�״̬�������û������ڣ��򷵻�0
    int32_t getUserStatusByUserId(int32_t userid);
    // ��ȡ�û��ͻ������ͣ�������û������ڣ��򷵻�0
    int32_t getUserClientTypeByUserId(int32_t userid);

   private:
    // �����ӵ������û����ӶϿ���������Ҫͨ��conn->connected()���жϣ�һ��ֻ����loop�������
    void onConnected(std::shared_ptr<TcpConnection> conn);
    // ���ӶϿ�
    void onDisconnected(const std::shared_ptr<TcpConnection>& conn);

   private:
    std::unique_ptr<TcpServer>              m_server;
    std::list<std::shared_ptr<ChatSession>> m_sessions;
    std::mutex                              m_sessionMutex; // ���߳�֮�䱣��m_sessions
    std::atomic_int                         m_sessionId{};
    std::mutex                              m_idMutex; // ���߳�֮�䱣��m_baseUserId
    std::atomic_bool m_logPackageBinary; // �Ƿ���־��ӡ�����Ķ���������
};
