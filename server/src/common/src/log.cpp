#include "log.h"
// #include <ctime>
// #include <time.h>
// #include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <sstream>

#define MAX_LOG_LINE_LENGTH (256)
#define DEFAULT_ROLL_SIZE (10 * 1024 * 1024)

bool WhispLog::log_init(const char* log_file_name, bool truncate_flag, u_int64_t roll_size)
{
    truncate_flag_ = truncate_flag;
    roll_size_     = roll_size;

    if (nullptr == log_file_name || log_file_name[0] == 0) {
        log_name_.clear();
    } else {
        log_name_ = log_file_name;
    }

    // 获取进程ID
    char pid[8];
    snprintf(pid, sizeof(pid), "%05d", static_cast<int>(getpid()));
    log_id_ = pid;

    log_file_     = nullptr;
    cur_level_    = LOG_LEVEL_INFO;
    writen_size_  = 0;
    exit_flag_    = false;
    running_flag_ = false;

    // write_thread_pool_.reset(new std::thread(write_thread_proc));
    write_thread_pool_ = std::make_unique<std::thread>([this]() { this->write_thread_proc(); });

    return true;
}

void WhispLog::log_uninit()
{
    running_flag_ = false;
    exit_flag_    = true;
    write_cond_.notify_one();

    if (write_thread_pool_->joinable()) write_thread_pool_->join();

    if (nullptr != log_file_) {
        fclose(log_file_);
        log_file_ = nullptr;
    }
}

void WhispLog::log_set_level(LOG_LEVEL levelv)
{
    if (levelv < LOG_LEVEL_TRACE || levelv > LOG_LEVEL_FATAL) {
        std::cout << "set log level error(%d)" << levelv << std::endl;
        return;
    }
    cur_level_ = levelv;

    return;
}

bool WhispLog::log_isrunning()
{
    return running_flag_;
}

bool WhispLog::log_output(LOG_LEVEL levelv, const char* fmt, ...)
{
    if (levelv < cur_level_ && levelv != LOG_LEVEL_CRITICAL) return false;

    std::string output_line;
    set_line_perfix(levelv, output_line);

    // 计算不定参数长度，以便分配空间
    va_list ap;
    va_start(ap, fmt);
    int msg_len = vsnprintf(nullptr, 0, fmt, ap);
    va_end(ap);

    if (msg_len < 0) {
        // 处理错误情况，vsnprintf 可能返回负数，表示错误
        return false;
    }

    // 为格式化后的消息分配足够的空间
    std::string log_msg(static_cast<std::string::size_type>(msg_len) + 1, '\0'); // +1 是为了包含 '\0'

    // 使用 vsnprintf 格式化日志消息
    va_list aq;
    va_start(aq, fmt);
    vsnprintf(&log_msg[0], log_msg.size(), fmt, aq); // 使用 log_msg 的数据
    va_end(aq);

    // 构建日志输出
    output_line += log_msg;

    if (truncate_flag_) output_line = output_line.substr(0, MAX_LOG_LINE_LENGTH); // 截取最大长度

    if (!log_name_.empty()) {
        output_line += '\n';
    }

    if (levelv != LOG_LEVEL_FATAL) {
        std::lock_guard<std::mutex> lock_guard(write_mutex_);
        write_wating_lists_.push_back(output_line);
        write_cond_.notify_one();
    } else {
        // 为了让 FATAL 级别的日志能够立即崩溃程序，采取同步写日志
        std::cout << output_line << std::endl;

        if (!log_name_.empty()) {
            if (nullptr == log_file_) {
                // 没有文件句柄，新建文件
                char   timef[64];
                time_t cur_time = time(NULL);
                tm     time_info;

                localtime_r(&cur_time, &time_info);
                strftime(timef, sizeof(timef), "%Y%m%d%H%M%S", &time_info);

                std::string new_log_file(log_name_);
                new_log_file += ".";
                new_log_file += timef;
                new_log_file += ".";
                new_log_file += log_id_;
                new_log_file += ".log";
                if (!create_file(new_log_file.c_str())) {
                    std::cout << "create log file :" << new_log_file << " fail" << std::endl;
                    return false;
                }
            }
            write2file(output_line);
        }
        // crash();
    }
    return true;
}

bool WhispLog::log_output(LOG_LEVEL levelv, const char* file_name, int line_num, const char* fmt, ...)
{
    if (levelv < cur_level_ && levelv != LOG_LEVEL_CRITICAL) return false;

    std::string out_line;
    set_line_perfix(levelv, out_line);

    // 函数签名
    char file_info[512] = {0};
    snprintf(file_info, sizeof(file_info), "[%s:%d]", file_name, line_num);
    out_line += file_info;

    // log msg
    std::string log_msg;
    va_list     ap;
    va_start(ap, fmt);

    // 计算最终日志的长度
    int msg_len = vsnprintf(nullptr, 0, fmt, ap);
    va_end(ap);

    if (msg_len < 0) {
        // 处理错误情况，vsnprintf 可能会返回负数，表示错误
        return false;
    }

    log_msg.resize(static_cast<std::size_t>(msg_len) + 1); // +1 是为了包含 '\0'
    va_list aq;
    va_start(aq, fmt);
    vsnprintf(&log_msg[0], log_msg.capacity(), fmt, aq); // 使用 log_msg 的数据
    va_end(aq);

    // 将格式化后的消息追加到日志
    if (truncate_flag_) log_msg = log_msg.substr(0, MAX_LOG_LINE_LENGTH); // 截取最大长度

    out_line += log_msg;

    if (!log_name_.empty()) {
        out_line += '\n';
    }

    if (levelv != LOG_LEVEL_FATAL) {
        std::lock_guard<std::mutex> lock_guard(write_mutex_);
        write_wating_lists_.push_back(out_line);
        write_cond_.notify_one();
    } else {
        // 为了让 FATAL 级别的日志能够立即崩溃程序，采取同步写日志
        std::cout << out_line << std::endl;

        if (!log_name_.empty()) {
            if (nullptr == log_file_) {
                // 没有文件句柄，新建文件
                char   timef[64];
                time_t cur_time = time(NULL);
                tm     time_info;

                localtime_r(&cur_time, &time_info);
                strftime(timef, sizeof(timef), "%Y%m%d%H%M%S", &time_info);

                std::string new_log_file(log_name_);
                new_log_file += ".";
                new_log_file += timef;
                new_log_file += ".";
                new_log_file += log_id_;
                new_log_file += ".log";
                if (!create_file(new_log_file.c_str())) {
                    std::cout << "create log file :" << new_log_file << " fail" << std::endl;
                    return false;
                }
            }
            write2file(out_line);
        }
        // crash();
    }
    return true;
}

bool WhispLog::log_output_binary(unsigned char* buffer, size_t size)
{
    std::ostringstream os; // #include <sstream>

    static const size_t PRINT_SIZE = 512;
    char                tmpbuf[PRINT_SIZE * 3 + 8];
    size_t              lsize = 0, lprintbuf_size = 0;
    int                 index = 0;
    os << "address[" << reinterpret_cast<long>(buffer) << "] size[" << size << "] \n";

    while (true) {
        memset(tmpbuf, 0, sizeof(tmpbuf));

        if (size > lsize) {
            lprintbuf_size = (size - lsize) > PRINT_SIZE ? PRINT_SIZE : (size - lsize);
            form_log(index, tmpbuf, sizeof(tmpbuf), buffer + lsize, lprintbuf_size);
            // size_t len = strlen(tmpbuf);

            os << tmpbuf;
            lsize += lprintbuf_size;
        } else {
            break;
        }
    }

    std::lock_guard<std::mutex> lock_guard(write_mutex_);
    write_wating_lists_.push_back(os.str());
    write_cond_.notify_one();

    return true;
}

const char* WhispLog::ull2str(int n)
{
    static char tmpbuf[64 + 1]; // 这是单线程方式
    memset(tmpbuf, 0, sizeof(tmpbuf));
    sprintf(tmpbuf, "%06u", n);
    return tmpbuf;
}

char* WhispLog::form_log(int& index, char* buf, size_t buf_size, unsigned char* buffer, size_t size)
{
    if (!buf || buf_size == 0 || !buffer) return nullptr;

    size_t len           = 0;
    size_t lsize         = 0;
    char   magic_num[17] = "0123456789abcdef";

    while (size > lsize && len + 3 < buf_size) {
        if (lsize % 32 == 0) {
            if (lsize != 0 && len + 1 < buf_size) {
                buf[len++] = '\n';
            }

            char head_buf[64];
            // 修复：处理 snprintf 返回值并统一为 size_t
            int    raw_head_len = snprintf(head_buf, sizeof(head_buf), "%s ", ull2str(index++));
            size_t head_len     = (raw_head_len < 0)
                                      ? 0
                                      : static_cast<size_t>(std::min(raw_head_len, static_cast<int>(sizeof(head_buf) - 1)));

            if (len + head_len >= buf_size) break;
            memcpy(buf + len, head_buf, head_len);
            len += head_len;
        }

        if (lsize % 16 == 0 && lsize != 0 && len + 1 < buf_size) {
            buf[len++] = ' ';
        }

        if (len + 2 >= buf_size) break;
        buf[len++] = magic_num[(buffer[lsize] >> 4) & 0xF];
        buf[len++] = magic_num[buffer[lsize] & 0xF];
        lsize++;
    }

    if (len < buf_size) buf[len++] = '\n';
    buf[std::min(len, buf_size - 1)] = '\0';
    return buf;
}

void WhispLog::set_line_perfix(LOG_LEVEL levelv, std::string& prefix_str)
{
    // 级别
    prefix_str = "[INFO]";
    if (levelv == LOG_LEVEL_TRACE)
        prefix_str = "[TRACE]";
    else if (levelv == LOG_LEVEL_DEBUG)
        prefix_str = "[DEBUG]";
    else if (levelv == LOG_LEVEL_WARNING)
        prefix_str = "[WARN]";
    else if (levelv == LOG_LEVEL_ERROR)
        prefix_str = "[ERROR]";
    else if (levelv == LOG_LEVEL_SYSERROR)
        prefix_str = "[SYSE]";
    else if (levelv == LOG_LEVEL_FATAL)
        prefix_str = "[FATAL]";
    else if (levelv == LOG_LEVEL_CRITICAL)
        prefix_str = "[CRITICAL]";

    // 时间
    char szTime[64] = {0};
    get_time(szTime, sizeof(szTime));

    prefix_str += "[";
    prefix_str += szTime;
    prefix_str += "]";

    // 当前线程信息
    char               thread_id[32] = {0};
    std::ostringstream osThreadID;
    osThreadID << std::this_thread::get_id();
    snprintf(thread_id, sizeof(thread_id), "[%s]", osThreadID.str().c_str());
    prefix_str += thread_id;
}

void WhispLog::get_time(char* ts, size_t ts_len)
{
    struct timeval tv;
    gettimeofday(&tv, nullptr); // 获取当前时间（秒 & 微秒）

    struct tm time;
    localtime_r(&tv.tv_sec, &time); // 转换为本地时间

    // 格式化输出时间（精确到毫秒）
    snprintf(ts, ts_len, "[%04d-%02d-%02d %02d:%02d:%02d:%03ld]", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
             time.tm_hour, time.tm_min, time.tm_sec,
             tv.tv_usec / 1000); // 微秒(us) 转换为 毫秒(ms)
}

bool WhispLog::create_file(const char* log_file_name)
{
    if (nullptr != log_file_) {
        fclose(log_file_);
    }

    log_file_ = fopen(log_file_name, "w+");
    return nullptr != log_file_;
}

bool WhispLog::write2file(const std::string& data)
{
    std::string tmp(data);
    size_t      ret = 0;
    while (true) {
        ret = fwrite(tmp.c_str(), 1, tmp.length(), log_file_);
        if (ret < tmp.length()) { // 写入失败，不完全
            return false;
        }
        if (ret == tmp.length()) {
            tmp.erase(0, ret); // 删除tmp第 0-ret 的字符
        }

        if (tmp.empty()) break;
    }
    fflush(log_file_);

    return true;
}

void WhispLog::crash() // dump掉
{
    char* p = nullptr;
    *p      = 0;
}

void WhispLog::write_thread_proc()
{
    running_flag_ = true;

    while (true) {
        if (!log_name_.empty()) {
            if (nullptr == log_file_ || writen_size_ >= roll_size_) {
                writen_size_ = 0;

                // 第一次或者文件大小超过roll size，均新建文件
                char   timef[64];
                time_t cur_time = time(NULL);
                tm     time_info;

                localtime_r(&cur_time, &time_info);
                strftime(timef, sizeof(timef), "%Y%m%d%H%M%S", &time_info);

                std::string new_log_file(log_name_);
                new_log_file += ".";
                new_log_file += timef;
                new_log_file += ".";
                new_log_file += log_id_;
                new_log_file += ".log";
                if (!create_file(new_log_file.c_str())) {
                    std::cout << "creat log file :" << new_log_file << " fail" << std::endl;
                    return;
                }
            }
        }

        std::string                  out_line;
        std::unique_lock<std::mutex> guard(write_mutex_);
        while (write_wating_lists_.empty()) {
            if (exit_flag_) return;

            write_cond_.wait(guard);
        }

        out_line = write_wating_lists_.front();
        write_wating_lists_.pop_front();

        std::cout << out_line << std::endl;

        if (!log_name_.empty()) {
            if (!write2file(out_line)) return;

            writen_size_ += out_line.length();
        }
    }

    running_flag_ = false;
}