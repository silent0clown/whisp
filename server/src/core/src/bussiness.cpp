/**
 * ��ʱͨѶ��ҵ���߼���ͳһ�������BusinessLogic.cpp
 * zhangyl 2018.05.16
 */

#include "bussiness.h"
#include <string>
// #include "json/json.h"
#include "json/reader.h"
#include "log.h"
// #include "session/chat_server.h"
#include "singleton.h"
#include "tcp_connection.h"
#include "user_manager.h"
void BussinessLogic::registerUser(const std::string& data, const std::shared_ptr<TcpConnection>& conn, bool keepalive,
                                  std::string& retData)
{
    //{ "user": "13917043329", "nickname" : "balloon", "password" : "123" }
    (void) keepalive;
    Json::CharReaderBuilder b;
    Json::CharReader*       reader(b.newCharReader());
    Json::Value             jsonRoot;
    JSONCPP_STRING          errs;
    bool                    ok = reader->parse(data.c_str(), data.c_str() + data.length(), &jsonRoot, &errs);
    if (!ok || errs.size() != 0) {
        LOG_WARN("invalid json: %s, client: %s", data.c_str(), conn->peerAddress().toIpPort().c_str());
        delete reader;
        return;
    }
    delete reader;

    if (!jsonRoot["username"].isString() || !jsonRoot["nickname"].isString() || !jsonRoot["password"].isString()) {
        LOG_WARN("invalid json: %s, client: %s", data.c_str(), conn->peerAddress().toIpPort().c_str());
        return;
    }

    User u;
    u.username = jsonRoot["username"].asString();
    u.nickname = jsonRoot["nickname"].asString();
    u.password = jsonRoot["password"].asString();

    // std::string retData;
    User cachedUser;
    cachedUser.userid = 0;
    Singleton<UserManager>::Instance().getUserInfoByUsername(u.username, cachedUser);
    if (cachedUser.userid != 0)
        retData = "{\"code\": 101, \"msg\": \"registered already\"}";
    else {
        if (!Singleton<UserManager>::Instance().addUser(u))
            retData = "{\"code\": 100, \"msg\": \"register failed\"}";
        else {
            retData = "{\"code\": 0, \"msg\": \"ok\"}";
        }
    }

    // conn->Send(msg_type_register, m_seq, retData);

    // LOGI << "Response to client: cmd=msg_type_register" << ", userid=" << u.userid << ", data=" << retData;
}