/**
 * ChatSession.h
 * zhangyl, 2017.03.10
 **/

#pragma once
#include "byte_buffer.h"
#include "tcp_session.h"
#include "timer_id.h"
using namespace network;

struct OnlineUserInfo {
    int32_t     userid;
    std::string username;
    std::string nickname;
    std::string password;
    int32_t     clienttype; // �ͻ�������, 0δ֪, pc=1, android/ios=2
    int32_t     status;     // ����״̬ 0���� 1���� 2æµ 3�뿪 4����
};

/**
 * ����Ự��
 */
class ChatSession : public TcpSession {
   public:
    ChatSession(const std::shared_ptr<TcpConnection>& conn, int sessionid);
    virtual ~ChatSession();

    ChatSession(const ChatSession& rhs)            = delete;
    ChatSession& operator=(const ChatSession& rhs) = delete;

    // �����ݿɶ�, �ᱻ�������loop����
    void onRead(const std::shared_ptr<TcpConnection>& conn, ByteBuffer* pBuffer, Timestamp receivTime);

    int32_t getSessionId()
    {
        return m_id;
    }

    int32_t getUserId()
    {
        return m_userinfo.userid;
    }

    std::string getUsername()
    {
        return m_userinfo.username;
    }

    std::string getNickname()
    {
        return m_userinfo.nickname;
    }

    std::string getPassword()
    {
        return m_userinfo.password;
    }

    int32_t getClientType()
    {
        return m_userinfo.clienttype;
    }

    int32_t getUserStatus()
    {
        return m_userinfo.status;
    }

    int32_t getUserClientType()
    {
        return m_userinfo.clienttype;
    }

    /**
     *@param type ȡֵ�� 1 �û����ߣ� 2 �û����ߣ� 3 �����ǳơ�ͷ��ǩ������Ϣ����
     */
    void sendUserStatusChangeMsg(int32_t userid, int type, int status = 0);

    // ��SessionʧЧ�����ڱ������ߵ��û���session
    void makeSessionInvalid();
    bool isSessionValid();

    void enableHearbeatCheck();
    void disableHeartbeatCheck();

    // ��������������ָ��ʱ���ڣ�������30�룩δ�յ����ݰ����������Ͽ��ڿͻ��˵�����
    void checkHeartbeat(const std::shared_ptr<TcpConnection>& conn);

   private:
    bool process(const std::shared_ptr<TcpConnection>& conn, const char* inbuf, size_t buflength);

    void onHeartbeatResponse(const std::shared_ptr<TcpConnection>& conn);
    void onRegisterResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void onLoginResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void onGetFriendListResponse(const std::shared_ptr<TcpConnection>& conn);
    void onFindUserResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void onChangeUserStatusResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void onOperateFriendResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void onAddGroupResponse(int32_t groupId, const std::shared_ptr<TcpConnection>& conn);
    void onUpdateUserInfoResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void onModifyPasswordResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void onCreateGroupResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void onGetGroupMembersResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void onChatResponse(int32_t targetid, const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void onMultiChatResponse(const std::string& targets, const std::string& data,
                             const std::shared_ptr<TcpConnection>& conn);
    void onScreenshotResponse(int32_t targetid, const std::string& bmpHeader, const std::string& bmpData,
                              const std::shared_ptr<TcpConnection>& conn);
    void onUpdateTeamInfoResponse(int32_t operationType, const std::string& newTeamName, const std::string& oldTeamName,
                                  const std::shared_ptr<TcpConnection>& con);
    void onModifyMarknameResponse(int32_t friendid, const std::string& newmarkname,
                                  const std::shared_ptr<TcpConnection>& conn);
    void onMoveFriendToOtherTeamResponse(int32_t friendid, const std::string& newteamname,
                                         const std::string& oldteamname, const std::shared_ptr<TcpConnection>& conn);

    void deleteFriend(const std::shared_ptr<TcpConnection>& conn, int32_t friendid);

    // �����û�������Ϣ��װӦ����ͻ��˵ĺ����б���Ϣ
    void makeUpFriendListInfo(std::string& friendinfo, const std::shared_ptr<TcpConnection>& conn);

    // ��������Ϣ�ı���ʱ��ĳɷ�����ʱ�䣬�޸ĳɹ�����true,ʧ�ܷ���false��
    bool modifyChatMsgLocalTimeToServerTime(const std::string& chatInputJson, std::string& chatOutputJson);

   private:
    int32_t        m_id; // session id
    OnlineUserInfo m_userinfo;
    int32_t        m_seq;                // ��ǰSession���ݰ����к�
    bool           m_isLogin;            // ��ǰSession��Ӧ���û��Ƿ��Ѿ���¼
    time_t         m_lastPackageTime;    // ��һ���շ�����ʱ��
    TimerId        m_checkOnlineTimerId; // ����Ƿ����ߵĶ�ʱ��id
};