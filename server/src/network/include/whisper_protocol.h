// whisper_protocol.h
#pragma once

#include <cstdint>
#include <string_view>

namespace whisker::protocol {

// 协议版本号
constexpr uint16_t PROTOCOL_MAJOR_VERSION = 1;
constexpr uint16_t PROTOCOL_MINOR_VERSION = 0;

// 消息类型（按功能域划分）
enum class MessageType : uint16_t {
    // 控制消息 (0-999)
    UNKNOWN         = 0,
    HEARTBEAT       = 1,

    // 认证相关 (1000-1099)
    REGISTER        = 1000,
    LOGIN           = 1001,
    LOGOUT          = 1002,

    // 好友操作 (1100-1199)
    FRIEND_REQUEST  = 1100,
    FRIEND_ACCEPT   = 1101,
    FRIEND_REMOVE   = 1102,

    // 聊天消息 (2000-2999)
    CHAT_TEXT       = 2000,
    CHAT_IMAGE      = 2001,
    CHAT_VOICE      = 2002
};

// 在线状态
enum class OnlineStatus : uint8_t {
    OFFLINE         = 0,
    PC_ONLINE       = 1,
    MOBILE_WIFI     = 2,
    MOBILE_DATA     = 3
};

#pragma pack(push, 1)
// 协议头（总长度24字节）
struct MessageHeader {
    uint32_t magic;          // 魔数 0x57484953 ("WHIS")
    uint16_t version;        // 协议版本
    uint16_t msg_type;       // MessageType值
    uint32_t body_length;    // 消息体长度（网络字节序）
    uint64_t timestamp;      // 毫秒级时间戳
    uint32_t checksum;       // 头部的CRC32校验
};
#pragma pack(pop)

// 校验头部的有效性
bool ValidateHeader(const MessageHeader& header);

} // namespace whisker::protocol