/**
 *  ��Ϣ�����࣬ MsgCacheManager.h
 *  zhangyl 2017.03.16
 **/
#pragma once
#include <stdint.h>
#include <list>
#include <mutex>
#include <string>

struct NotifyMsgCache {
    int32_t     userid;
    std::string notifymsg;
};

struct ChatMsgCache {
    int32_t     userid;
    std::string chatmsg;
};

class MsgCacheManager final {
   public:
    MsgCacheManager();
    ~MsgCacheManager();

    MsgCacheManager(const MsgCacheManager& rhs)            = delete;
    MsgCacheManager& operator=(const MsgCacheManager& rhs) = delete;

    bool addNotifyMsgCache(int32_t userid, const std::string& cache);
    void getNotifyMsgCache(int32_t userid, std::list<NotifyMsgCache>& cached);

    bool addChatMsgCache(int32_t userid, const std::string& cache);
    void getChatMsgCache(int32_t userid, std::list<ChatMsgCache>& cached);

   private:
    std::list<NotifyMsgCache> m_listNotifyMsgCache; // ֪ͨ����Ϣ���棬����Ӻ�����Ϣ
    std::mutex                m_mtNotifyMsgCache;
    std::list<ChatMsgCache>   m_listChatMsgCache; // ������Ϣ����
    std::mutex                m_mtChatMsgCache;
};
