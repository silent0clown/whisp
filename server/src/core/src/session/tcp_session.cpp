/**
 * TcpSession.cpp
 * zhangyl 2017.03.09
 **/
#include "tcp_session.h"
#include "chat_server.h"
#include "log.h"
#include "msg.h"
#include "protocol_stream.h"
#include "singleton.h"
#include "zlib_util.h"

TcpSession::TcpSession(const std::weak_ptr<TcpConnection>& tmpconn) : tmpConn_(tmpconn) {}

TcpSession::~TcpSession() {}

void TcpSession::send(int32_t cmd, int32_t seq, const std::string& data)
{
    send(cmd, seq, data.c_str(), static_cast<int32_t>(data.length()));
}

void TcpSession::send(int32_t cmd, int32_t seq, const char* data, int32_t dataLength)
{
    std::string                 outbuf;
    network::BinaryStreamWriter writeStream(&outbuf);
    writeStream.WriteInt32(cmd);
    writeStream.WriteInt32(seq);
    writeStream.WriteCString(data, static_cast<size_t>(dataLength));
    writeStream.Flush();

    sendPackage(outbuf.c_str(), static_cast<int32_t>(outbuf.length()));
}

void TcpSession::send(const std::string& p)
{
    sendPackage(p.c_str(), static_cast<int32_t>(p.length()));
}

void TcpSession::send(const char* p, int32_t length)
{
    sendPackage(p, length);
}

void TcpSession::sendPackage(const char* p, int32_t length)
{
    string srcbuf(p, static_cast<std::string::size_type>(length));
    string destbuf;
    if (!ZlibUtil::compressBuf(srcbuf, destbuf)) {
        LOG_ERROR("compress buf error");
        return;
    }

    string          strPackageData;
    chat_msg_header header;
    header.compressflag = 1;
    header.compresssize = static_cast<int32_t>(destbuf.length());
    header.originsize   = length;
    if (Singleton<ChatServer>::Instance().isLogPackageBinaryEnabled()) {
        LOG_INFO("Send data, header length: %d, body length: %d", sizeof(header), destbuf.length());
    }

    // ����һ����ͷ
    strPackageData.append(reinterpret_cast<const char*>(&header), sizeof(header));
    strPackageData.append(destbuf);

    // TODO: ��ЩSession��connection�������������Ҫ�ú�����һ��
    if (tmpConn_.expired()) {
        // FIXME: ��������������Ҫ�Ų�
        LOG_ERROR("Tcp connection is destroyed , but why TcpSession is still alive ?");
        return;
    }

    std::shared_ptr<TcpConnection> conn = tmpConn_.lock();
    if (conn) {
        if (Singleton<ChatServer>::Instance().isLogPackageBinaryEnabled()) {
            size_t packageLength = strPackageData.length();
            LOG_INFO("Send data, package length: %d", packageLength);
            // LOG_DEBUG_BIN((unsigned char*)strPackageData.c_str(), packageLength);
        }

        conn->send(strPackageData);
    }
}