#include "binary_codec.h"
#include "zlib_util.h"  // 压缩工具封装
#include <crc32.h>      // CRC校验实现

namespace whisker::protocol {

namespace {
    size_t compression_threshold_ = 1024;
}

std::shared_ptr<ByteBuffer> BinaryCodec::Encode(
    MessageType type,
    const std::string_view body,
    bool compress) 
{
    auto buffer = std::make_shared<ByteBuffer>();
    
    // 准备消息头
    MessageHeader header{};
    header.magic = 0x57484953;
    header.version = (PROTOCOL_MAJOR_VERSION << 8) | PROTOCOL_MINOR_VERSION;
    header.msg_type = static_cast<uint16_t>(type);
    header.timestamp = GetCurrentMillis();

    // 处理消息体
    std::string processed_body(body);
    if (compress && body.size() > compression_threshold_) {
        if (auto compressed = ZlibUtil::Compress(body)) {
            processed_body = std::move(*compressed);
            header.body_length = htonl(processed_body.size());
        }
    } else {
        header.body_length = htonl(body.size());
    }

    // 计算头部校验
    header.checksum = crc32(
        reinterpret_cast<const uint8_t*>(&header),
        sizeof(header) - sizeof(uint32_t));

    // 写入缓冲区
    buffer->append(&header, sizeof(header));
    buffer->append(processed_body.data(), processed_body.size());

    return buffer;
}

std::optional<std::pair<MessageType, std::string>> 
BinaryCodec::Decode(const ByteBuffer& buffer) 
{
    if (buffer.readableBytes() < sizeof(MessageHeader)) {
        return std::nullopt;
    }

    // 读取并验证头部
    MessageHeader header;
    buffer.peek(&header, sizeof(header));

    if (header.magic != 0x57484953 || 
        !ValidateHeader(header) ||
        buffer.readableBytes() < sizeof(header) + ntohl(header.body_length)) 
    {
        return std::nullopt;
    }

    // 提取消息体
    auto body_start = buffer.begin() + sizeof(header);
    std::string body(body_start, body_start + ntohl(header.body_length));

    // 处理压缩（根据消息类型判断）
    if (IsCompressedMessage(static_cast<MessageType>(header.msg_type))) {
        if (auto decompressed = ZlibUtil::Decompress(body)) {
            body = std::move(*decompressed);
        } else {
            return std::nullopt;
        }
    }

    return std::make_pair(
        static_cast<MessageType>(header.msg_type),
        std::move(body)
    );
}

} // namespace whisker::protocol