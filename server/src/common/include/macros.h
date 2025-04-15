/*
* macros.h 应该集中管理 全项目通用的预处理宏和编译期元编程工具
    - 编译器/平台适配宏
    - 代码生成控制（禁用拷贝等）
    - 调试开发工具
    - 性能优化提示
    - 元编程工具
*/

// src/common/include/macros.h
#pragma once

// ==================== 编译器特性检测 ====================
#if defined(__clang__)
#define WHISKER_COMPILER_CLANG 1
#elif defined(__GNUC__)
#define WHISKER_COMPILER_GCC 1
#elif defined(_MSC_VER)
#define WHISKER_COMPILER_MSVC 1
#endif

// ==================== 平台特性宏 ====================
#if __cplusplus >= 201703L
#define WHISKER_NODISCARD [[nodiscard]]
#else
#define WHISKER_NODISCARD __attribute__((warn_unused_result))
#endif

#if defined(_WIN32) || defined(_WIN64)
    #define WHISP_PLATFORM_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
    #define WHISP_PLATFORM_MACOS
#elif defined(__linux__)
    #define WHISP_PLATFORM_LINUX
#else
    #define WHISP_PLATFORM_UNKNOWN
#endif

// ==================== 代码生成控制 ====================
// 禁用拷贝/移动语义
#define DISABLE_COPY(Class) \
    Class(const Class&) = delete; \
    Class& operator=(const Class&) = delete

#define DISABLE_MOVE(Class) \
    Class(Class&&) = delete; \
    Class& operator=(Class&&) = delete

// ==================== 调试辅助 ====================
#ifdef NDEBUG
#define WHISKER_DEBUG_BREAK() do {} while (0)
#define WHISKER_ASSERT(expr) do {} while (0)
#else
#define WHISKER_DEBUG_BREAK() \
    do { \
        volatile bool* trap = nullptr; \
        *trap = true; \
    } while (0)

#define WHISKER_ASSERT(expr) \
    do { \
        if (!(expr)) { \
            ::whisker::log::Error("Assert failed: " #expr); \
            WHISKER_DEBUG_BREAK(); \
        } \
    } while (0)
#endif

// ==================== 性能优化 ====================
// 强制内联
#if WHISKER_COMPILER_MSVC
#define FORCE_INLINE __forceinline
#else
#define FORCE_INLINE inline __attribute__((always_inline))
#endif

// 热路径标记
#define WHISKER_HOT_PATH [[hot]]

// ==================== 跨平台兼容 ====================
#if defined(_WIN32)
#define WHISKER_DLL_EXPORT __declspec(dllexport)
#define WHISKER_DLL_IMPORT __declspec(dllimport)
#else
#define WHISKER_DLL_EXPORT __attribute__((visibility("default")))
#define WHISKER_DLL_IMPORT
#endif

// ==================== 元编程工具 ====================
// 编译时数组大小计算
// template<typename T, size_t N>
// constexpr size_t ARRAY_SIZE(T (&)[N]) { return N; }

// 禁用构造函数
#define NO_DEFAULT_CTOR(Class) Class() = delete


// ==================== 编译模式 ====================
#define WHISP_BUILD_MODE_DEBUG          (0)
#define WHISP_BUILD_MODE_RELEASE        (1)
#define WHISP_BUILD_MODE_TEST           (2)
