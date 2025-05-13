/**
 *  ��Ϣ�����࣬ MsgCacheManager.cpp
 *  zhangyl 2017.03.16
 **/
#include "msg_cache_manager.h"
#include "log.h"

MsgCacheManager::MsgCacheManager() {}

MsgCacheManager::~MsgCacheManager() {}

bool MsgCacheManager::addNotifyMsgCache(int32_t userid, const std::string& cache)
{
    std::lock_guard<std::mutex> guard(m_mtNotifyMsgCache);
    NotifyMsgCache              nc;
    nc.userid = userid;
    nc.notifymsg.append(cache.c_str(), cache.length());
    ;
    m_listNotifyMsgCache.push_back(nc);
    LOG_INFO("append notify msg to cache, userid: %d, , m_mapNotifyMsgCache.size(): %d, cache length: %d", userid,
             m_listNotifyMsgCache.size(), cache.length());

    // TODO: ���̻�д�����ݿ��Է�ֹ���������ʧ

    return true;
}

void MsgCacheManager::getNotifyMsgCache(int32_t userid, std::list<NotifyMsgCache>& cached)
{
    std::lock_guard<std::mutex> guard(m_mtNotifyMsgCache);
    for (auto iter = m_listNotifyMsgCache.begin(); iter != m_listNotifyMsgCache.end();) {
        if (iter->userid == userid) {
            cached.push_back(*iter);
            iter = m_listNotifyMsgCache.erase(iter);
        } else {
            iter++;
        }
    }

    LOG_INFO("get notify msg cache, userid: %d, , m_mapNotifyMsgCache.size(): %d, cached size: %d", userid,
             m_listNotifyMsgCache.size(), cached.size());
}

bool MsgCacheManager::addChatMsgCache(int32_t userid, const std::string& cache)
{
    std::lock_guard<std::mutex> guard(m_mtChatMsgCache);
    ChatMsgCache                c;
    c.userid = userid;
    c.chatmsg.append(cache.c_str(), cache.length());
    m_listChatMsgCache.push_back(c);
    LOG_INFO("append chat msg to cache, userid: %d, m_listChatMsgCache.size(): , cache length: %d", userid,
             m_listChatMsgCache.size(), cache.length());
    // TODO: ���̻�д�����ݿ��Է�ֹ���������ʧ

    return true;
}

void MsgCacheManager::getChatMsgCache(int32_t userid, std::list<ChatMsgCache>& cached)
{
    std::lock_guard<std::mutex> guard(m_mtChatMsgCache);
    for (auto iter = m_listChatMsgCache.begin(); iter != m_listChatMsgCache.end();) {
        if (iter->userid == userid) {
            cached.push_back(*iter);
            iter = m_listChatMsgCache.erase(iter);
        } else {
            iter++;
        }
    }

    LOG_INFO("get chat msg cache, userid: %d, m_listChatMsgCache.size(): %d, cached size: %d", userid,
             m_listChatMsgCache.size(), cached.size());
}