#pragma once

#ifdef _WIN32
// Windows特有的头文件和定义
#include <winsock2.h>
typedef unsigned long in_addr_t; // 如果没有定义in_addr_t，则手动定义
#else
// Unix/Linux/Mac特有的头文件
#include <arpa/inet.h>
#include <netinet/in.h>
#endif
#include <cstdint> // For uint16_t and uint32_t
#include <string>
// #include "../base/Platform.h"

namespace network {
class InetAddress {
   public:
    explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false);

    /// @param ip should be "1.2.3.4"
    InetAddress(const std::string& ip, uint16_t port);

    InetAddress(const struct sockaddr_in& addr) : addr_(addr) {}

    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t    toPort() const;

    const struct sockaddr_in& getSockAddrInet() const
    {
        return addr_;
    }
    void setSockAddrInet(const struct sockaddr_in& addr)
    {
        addr_ = addr;
    }

    uint32_t ipNetEndian() const
    {
        return addr_.sin_addr.s_addr;
    }
    uint16_t portNetEndian() const
    {
        return addr_.sin_port;
    }

    static bool resolve(const std::string& hostname, InetAddress* result);

   private:
    struct sockaddr_in addr_;
};

} // namespace network