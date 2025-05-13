/**
 *  �������������࣬IMServer.cpp
 *  zhangyl 2017.03.09
 **/
#include "session/chat_server.h"

#include "chat_session.h"
#include "inet_address.h"
#include "log.h"
#include "singleton.h"
#include "user_manager.h"

ChatServer::ChatServer()
{
    m_logPackageBinary = false;
}

bool ChatServer::init(const char* ip, short port, EventLoop* loop)
{
    InetAddress addr(ip, static_cast<uint16_t>(port));
    m_server.reset(new TcpServer(loop, addr, "FLAMINGO-SERVER", TcpServer::kReusePort));
    m_server->setConnectionCallback(std::bind(&ChatServer::onConnected, this, std::placeholders::_1));
    // ��������
    m_server->start(6);

    return true;
}

void ChatServer::uninit()
{
    if (m_server) m_server->stop();
}

void ChatServer::enableLogPackageBinary(bool enable)
{
    m_logPackageBinary = enable;
}

bool ChatServer::isLogPackageBinaryEnabled()
{
    return m_logPackageBinary;
}

void ChatServer::onConnected(std::shared_ptr<TcpConnection> conn)
{
    if (conn->connected()) {
        LOG_DEBUG("client connected: %s", conn->peerAddress().toIpPort().c_str());
        ++m_sessionId;
        std::shared_ptr<ChatSession> spSession(new ChatSession(conn, m_sessionId));
        conn->setMessageCallback(std::bind(&ChatSession::onRead, spSession.get(), std::placeholders::_1,
                                           std::placeholders::_2, std::placeholders::_3));

        std::lock_guard<std::mutex> guard(m_sessionMutex);
        m_sessions.push_back(spSession);
    } else {
        onDisconnected(conn);
    }
}

void ChatServer::onDisconnected(const std::shared_ptr<TcpConnection>& conn)
{
    // �Ƿ����û�����
    // bool bUserOffline = false;
    UserManager& userManager = Singleton<UserManager>::Instance();

    // TODO: �����Ĵ����߼�̫���ң���Ҫ�Ż�
    std::lock_guard<std::mutex> guard(m_sessionMutex);
    for (auto iter = m_sessions.begin(); iter != m_sessions.end(); ++iter) {
        if ((*iter)->getConnectionPtr() == NULL) {
            LOG_ERROR("connection is NULL");
            break;
        }

        // ͨ���ȶ�connection�����ҵ���Ӧ��session
        if ((*iter)->getConnectionPtr() == conn) {
            // ��Session����֮ǰ�������ߵ���ЧSession������Ϊ���������ߣ��Ÿ������������������Ϣ
            if ((*iter)->isSessionValid()) {
                // ���������ߺ��ѣ��������������������Ϣ
                std::list<User> friends;
                int32_t         offlineUserId = (*iter)->getUserId();
                userManager.getFriendInfoByUserId(offlineUserId, friends);
                for (const auto& iter2 : friends) {
                    for (auto& iter3 : m_sessions) {
                        // �ú����Ƿ����ߣ����߻����session��
                        if (iter2.userid == iter3->getUserId()) {
                            iter3->sendUserStatusChangeMsg(offlineUserId, 2);

                            LOG_INFO("SendUserStatusChangeMsg to user(userid=%d): user go offline, offline userid = %d",
                                     iter3->getUserId(), offlineUserId);
                        }
                    }
                }
            } else {
                LOG_INFO("Session is invalid, userid=%d", (*iter)->getUserId());
            }

            // ͣ����Session�ĵ��߼��
            //(*iter)->DisableHeartbaetCheck();
            // �û�����
            m_sessions.erase(iter);
            // bUserOffline = true;
            LOG_INFO("client disconnected: %s", conn->peerAddress().toIpPort().c_str());
            break;
        }
    }

    LOG_INFO("current online user count: %d", static_cast<int>(m_sessions.size()));
}

void ChatServer::getSessions(std::list<std::shared_ptr<ChatSession>>& sessions)
{
    std::lock_guard<std::mutex> guard(m_sessionMutex);
    sessions = m_sessions;
}

bool ChatServer::getSessionByUserIdAndClientType(std::shared_ptr<ChatSession>& session, int32_t userid,
                                                 int32_t clientType)
{
    std::lock_guard<std::mutex>  guard(m_sessionMutex);
    std::shared_ptr<ChatSession> tmpSession;
    for (const auto& iter : m_sessions) {
        tmpSession = iter;
        if (iter->getUserId() == userid && iter->getClientType() == clientType) {
            session = tmpSession;
            return true;
        }
    }

    return false;
}

bool ChatServer::getSessionsByUserId(std::list<std::shared_ptr<ChatSession>>& sessions, int32_t userid)
{
    std::lock_guard<std::mutex>  guard(m_sessionMutex);
    std::shared_ptr<ChatSession> tmpSession;
    for (const auto& iter : m_sessions) {
        tmpSession = iter;
        if (iter->getUserId() == userid) {
            sessions.push_back(tmpSession);
            return true;
        }
    }

    return false;
}

int32_t ChatServer::getUserStatusByUserId(int32_t userid)
{
    std::lock_guard<std::mutex> guard(m_sessionMutex);
    for (const auto& iter : m_sessions) {
        if (iter->getUserId() == userid) {
            return iter->getUserStatus();
        }
    }

    return 0;
}

int32_t ChatServer::getUserClientTypeByUserId(int32_t userid)
{
    std::lock_guard<std::mutex> guard(m_sessionMutex);
    bool                        bMobileOnline = false;
    int                         clientType    = CLIENT_TYPE_UNKOWN;
    for (const auto& iter : m_sessions) {
        if (iter->getUserId() == userid) {
            clientType = iter->getUserClientType();
            // ��������ֱ�ӷ��ص�������״̬
            if (clientType == CLIENT_TYPE_PC)
                return clientType;
            else if (clientType == CLIENT_TYPE_ANDROID || clientType == CLIENT_TYPE_IOS)
                bMobileOnline = true;
        }
    }

    // ֻ���ֻ����߲ŷ����ֻ�����״̬
    if (bMobileOnline) return clientType;

    return CLIENT_TYPE_UNKOWN;
}