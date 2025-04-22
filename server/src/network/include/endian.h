#pragma once
// endian.h
#include <arpa/inet.h>
#include <memory.h>
// #include <netinet/in.h>
#include <stdint.h>

static inline uint64_t hton64(uint64_t value)
{
    // 通过位移操作避免字节序依赖
    return (((value & 0xFF00000000000000ULL) >> 56) | ((value & 0x00FF000000000000ULL) >> 40) |
            ((value & 0x0000FF0000000000ULL) >> 24) | ((value & 0x000000FF00000000ULL) >> 8) |
            ((value & 0x00000000FF000000ULL) << 8) | ((value & 0x0000000000FF0000ULL) << 24) |
            ((value & 0x000000000000FF00ULL) << 40) | ((value & 0x00000000000000FFULL) << 56));
}

static inline uint64_t ntoh64(uint64_t value)
{
    // 使用memcpy避免严格别名问题（兼容所有编译器）
    uint64_t result;
    uint8_t  bytes[8] = {static_cast<uint8_t>(value >> 56), static_cast<uint8_t>(value >> 48),
                         static_cast<uint8_t>(value >> 40), static_cast<uint8_t>(value >> 32),
                         static_cast<uint8_t>(value >> 24), static_cast<uint8_t>(value >> 16),
                         static_cast<uint8_t>(value >> 8),  static_cast<uint8_t>(value)};
    memcpy(&result, bytes, sizeof(result));
    return result;
}

static inline uint16_t hton16(uint16_t value)
{
    return static_cast<uint16_t>(((value & 0x00FFU) << 8) | ((value & 0xFF00U) >> 8));
}

static inline uint16_t ntoh16(uint16_t value)
{
    return hton16(value); // 对称
}

static inline uint32_t hton32(uint32_t value)
{
    return static_cast<uint32_t>(((value & 0x000000FF00000000ULL) >> 32) | ((value & 0x00000000FF000000ULL) >> 8) |
                                 ((value & 0x0000000000FF0000ULL) << 8) | ((value & 0x000000000000FF00ULL) << 32));
}

static inline uint32_t ntoh32(uint32_t value)
{
    return hton32(value); // 对称
}

namespace network {
namespace sockets {
inline uint64_t hostToNetwork64(uint64_t host64)
{
#ifdef WIN32
    return htonll(host64);
#else
    return hton64(host64);
#endif
}

inline uint32_t hostToNetwork32(uint32_t host32)
{
#ifdef WIN32
    return htonl(host32);
#else
    return hton32(host32);
#endif
}

inline uint16_t hostToNetwork16(uint16_t host16)
{
#ifdef WIN32
    return htons(host16);
#else
    return hton16(host16);
#endif
}

inline uint64_t networkToHost64(uint64_t net64)
{
#ifdef WIN32
    return ntohll(net64);
#else
    return ntoh64(net64);
#endif
}

inline uint32_t networkToHost32(uint32_t net32)
{
#ifdef WIN32
    return ntohl(net32);
#else
    return ntoh32(net32);
#endif
}

inline uint16_t networkToHost16(uint16_t net16)
{
#ifdef WIN32
    return ntohs(net16);
#else
    return ntoh16(net16);
#endif
}
} // end namespace sockets
} // namespace network
