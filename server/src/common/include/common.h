#pragma once 
// common.h 应该包含 全项目共享的、与业务逻辑无关的基础定义
/*
    - 基础类型别名（统一项目中的类型表示）
    - 通用常量（全项目共享的数值约束）
    - 错误码（跨模块错误传递）
    - 平台宏（条件编译基础）
    - 工具宏（禁止拷贝/移动等）

避免放入 common.h 的内容:
    ❌ 模块专属定义（如网络协议、数据库类型）
    ❌ 函数实现（即使是工具函数）
    ❌ 第三方库头文件（如 <boost/asio.hpp>）
*/


#include <cstdint>
#include <string>
#include <memory>

namespace whisker {

// ==================== 基础类型别名 ====================
using Byte      = uint8_t;     // 明确字节类型
using Int64     = int64_t;      // 固定大小整数
using UInt64    = uint64_t;
using Timestamp = uint64_t;     // 毫秒级时间戳

// 智能指针模板简化
template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using WeakRef = std::weak_ptr<T>;

// ==================== 通用常量 ====================
namespace constants {
    constexpr size_t MAX_PACKET_SIZE    = 16 * 1024;  // 16KB
    constexpr int    DEFAULT_THREAD_NUM = 4;
} // namespace constants

// ==================== 错误码 ====================
enum class ErrorCode : uint16_t {
    OK                  = 0,
    INVALID_ARGUMENT    = 1001,
    NETWORK_ERROR       = 2001,
    DB_OPERATION_FAILED = 3001,
    // ... 其他错误码
};

// ==================== 平台兼容性 ====================
#if defined(_WIN32)
#define WHISKER_PLATFORM_WINDOWS
#elif defined(__linux__)
#define WHISKER_PLATFORM_LINUX
#endif

// ==================== 调试工具 ====================
#define DISABLE_COPY(ClassName) \
    ClassName(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&) = delete

#define MAKE_NONMOVABLE(ClassName) \
    ClassName(ClassName&&) = delete; \
    ClassName& operator=(ClassName&&) = delete

} // namespace whisker