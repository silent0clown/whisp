/**
 * Http�Ự��, HttpSession.cpp
 * zhangyl 2018.05.16
 */
#include "http_session.h"
#include <string.h>
#include <sstream>
#include <vector>
#include "bussiness.h"
// #include "chat_server.h"
// #include "eventloop_thread.h"
#include "log.h"
// #include "singleton.h"
#include "string_util.h"
#include "urlencode_util.h"
// #include "user_manager.h"

#define MAX_URL_LENGTH 2048

HttpSession::HttpSession(std::shared_ptr<TcpConnection>& conn) : m_tmpConn(conn) {}

void HttpSession::onRead(const std::shared_ptr<TcpConnection>& conn, ByteBuffer* pBuffer, Timestamp receivTime)
{
    // LOG_INFO << "Recv a http request from " << conn->peerAddress().toIpPort();

    (void) receivTime;
    string inbuf;
    // �Ȱ��������ݶ�ȡ����
    inbuf.append(pBuffer->peek(), pBuffer->readableBytes());
    // ��Ϊһ��http��ͷ����������\r\n\r\n�����Դ���4���ַ�
    // С�ڵ���4���ַ���˵������δ���꣬�˳����ȴ�����ײ������ȡ
    if (inbuf.length() <= 4) return;

    // �����յ���GET�������ݰ�һ���ʽ���£�
    /*
    GET
    /register.do?p={%22username%22:%20%2213917043329%22,%20%22nickname%22:%20%22balloon%22,%20%22password%22:%20%22123%22}
    HTTP/1.1\r\n Host: 120.55.94.78:12345\r\n Connection: keep-alive\r\n Upgrade-Insecure-Requests: 1\r\n User-Agent:
    Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/65.0.3325.146
    Safari/537.36\r\n Accept-Encoding: gzip, deflate\r\n Accept-Language: zh-CN, zh; q=0.9, en; q=0.8\r\n \r\n
     */
    // ����Ƿ���\r\n\r\n�������������˵����ͷ���������˳�
    string end = inbuf.substr(inbuf.length() - 4);
    if (end != "\r\n\r\n") return;
    // ����2048���ַ����Ҳ���\r\n\r\n��������Ϊ�ǷǷ�����
    else if (inbuf.length() >= MAX_URL_LENGTH) {
        conn->forceClose();
        return;
    }

    // ��\r\n�ָ�ÿһ��
    std::vector<string> lines;
    StringUtil::split(inbuf, lines, "\r\n");
    if (lines.size() < 1 || lines[0].empty()) {
        conn->forceClose();
        return;
    }

    std::vector<string> chunk;
    StringUtil::split(lines[0], chunk, " ");
    // chunk�������������ַ�����GET+url+HTTP�汾��
    if (chunk.size() < 3) {
        conn->forceClose();
        return;
    }

    LOG_INFO("url: %s  from %s", chunk[1].c_str(), conn->peerAddress().toIpPort().c_str());
    // inbuf =
    // /register.do?p={%22username%22:%20%2213917043329%22,%20%22nickname%22:%20%22balloon%22,%20%22password%22:%20%22123%22}
    std::vector<string> part;
    // ͨ��?�ָ��ǰ�����ˣ�ǰ����url�������ǲ���
    StringUtil::split(chunk[1], part, "?");
    // chunk�������������ַ�����GET+url+HTTP�汾��
    if (part.size() < 2) {
        conn->forceClose();
        return;
    }

    string url   = part[0];
    string param = part[1].substr(2);

    if (!process(conn, url, param)) {
        LOG_ERROR("handle http request error, from: %s, request: %s", conn->peerAddress().toIpPort().c_str(),
                  pBuffer->retrieveAllAsString().c_str());
    }

    // �����ӣ�������ر�����
    conn->forceClose();
}

void HttpSession::send(const char* data, size_t length)
{
    if (!m_tmpConn.expired()) {
        std::shared_ptr<TcpConnection> conn = m_tmpConn.lock();
        conn->send(data, static_cast<int>(length));
    }
}

bool HttpSession::process(const std::shared_ptr<TcpConnection>& conn, const std::string& url, const std::string& param)
{
    if (url.empty()) return false;

    if (url == "/register.do") {
        onRegisterResponse(param, conn);
    } else if (url == "/LOG_INFOn.do") {
        onLoginResponse(param, conn);
    } else if (url == "/getfriendlist.do") {
    } else if (url == "/getgroupmembers.do") {
    } else
        return false;

    return true;
}

// ��װhttpЭ��Ӧ���
/*
    HTTP/1.1 200 OK\r\n
    Content-Type: text/html\r\n
    Date: Wed, 16 May 2018 10:06:10 GMT\r\n
    Content-Length: 4864\r\n
    \r\n\
    {"code": 0, "msg": ok}
*/
void HttpSession::makeupResponse(const std::string& input, std::string& output)
{
    std::ostringstream os;
    os << "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:" << input.length() << "\r\n\r\n" << input;

    output = os.str();
}

void HttpSession::onRegisterResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn)
{
    string retData;
    string decodeData;
    URLEncodeUtil::decode(data, decodeData);
    BussinessLogic::registerUser(decodeData, conn, false, retData);
    if (!retData.empty()) {
        std::string response;
        URLEncodeUtil::encode(retData, response);
        makeupResponse(retData, response);
        conn->send(response);

        LOG_INFO("Response to client: cmd=msg_type_register, data: %s, client: %s", retData.c_str(),
                 conn->peerAddress().toIpPort().c_str());
    }
}

void HttpSession::onLoginResponse(const std::string& /*data*/, const std::shared_ptr<TcpConnection>& /*conn*/) {}