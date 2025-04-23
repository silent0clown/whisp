#pragma once

#include <string>
#include "log.h"
// #include <yaml-cpp/yaml.h>
// #include <yaml-cpp/yaml.h>
#define LOAD_CONFIG_VALUE(config_value, write_value)        \
    do {                                                    \
        if ((config_value).empty()) {                       \
            LOG_ERROR("[MAIN] Get " #config_value " fail"); \
            return 1;                                       \
        } else {                                            \
            (write_value) += (config_value);                \
        }                                                   \
    } while (0)

struct ClientConfig {
    short       client_listen_port;
    std::string client_listen_ip;
};

struct MonitorConfig {
    short       monitor_listen_port;
    std::string monitor_listen_ip;
    std::string monitor_token;
};

struct HttpConfig {
    short       http_listen_port;
    std::string http_listen_ip;
};

struct LogConfig {
    bool        log_binary_package;
    std::string log_file_dir;
    std::string log_file_name;
};

struct MysqlConfig {
    short       mysql_server_port;
    std::string mysql_server_addr;
    std::string user;
    std::string password;
    std::string database;
};

struct WhispConfig {
    struct ClientConfig  client_config;
    struct MonitorConfig monitor_config;
    struct HttpConfig    http_config;
    struct LogConfig     log_config;
    struct MysqlConfig   mysql_config;
};

class ConfigParser {
   public:
    // 入参1：配置文件路径， // 入参2：待加载配置项指针
    bool loadConfig(const std::string& file_name, struct WhispConfig& whisp_config);
    bool loadConfig(struct WhispConfig& whisp_config);
    void printConfig(const struct WhispConfig& whisp_config);

   private:
    std::string _getDefaultConfig();
};