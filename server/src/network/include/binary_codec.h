// 二进制编解码（主实现）
#pragma once

#include <memory>
#include <optional>
#include "whisper_protocol.h"

namespace whisker::protocol {

class BinaryCodec {
   public:
    // 编码消息到ByteBuffer
    static std::shared_ptr<ByteBuffer> Encode(MessageType type, const std::string_view body, bool compress = false);

    // 从ByteBuffer解码消息
    static std::optional<std::pair<MessageType, std::string>> Decode(const ByteBuffer& buffer);

    // 设置压缩阈值（默认1KB）
    static void SetCompressionThreshold(size_t threshold);
};

} // namespace whisker::protocol