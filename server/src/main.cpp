#include "config_parse.h"
#include "daemon_run.h"
#include "log.h"
// #include "macros.h"
#include "mysqlx_factory.h"
// #include "mysqlx_pool.h"
#include <string.h>
#include <iostream>
#include "chat_server.h"
#include "event_loop.h"
#include "http_server.h"
#include "monitor_server.h"
#include "singleton.h"
#include "user_manager.h"
#include "w_mysql/mysql_manager.h"
#if defined(__linux__)
#include <dirent.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#ifndef WIN32
void prog_exit(int signo)
{
    std::cout << "program recv signal [" << signo << "] to exit." << std::endl;

    // Singleton<MonitorServer>::Instance().uninit();
    // Singleton<HttpServer>::Instance().uninit();
    // Singleton<ChatServer>::Instance().uninit();
    // g_mainLoop.quit();

    // CAsyncLog::uninit();
}
#endif

network::EventLoop g_mainLoop;

// 参数解析
void parse_arguments(int argc, char* argv[])
{
    int  ch;
    bool bdaemon = false;
    while ((ch = getopt(argc, argv, "d")) != -1) {
        switch (ch) {
            case 'd':
                bdaemon = true;
                break;
        }
    }

    if (bdaemon) {
        std::cout << "server run in daemon mode." << std::endl;
        daemon_run();
    }
}

int main(int argc, char* argv[])
{
    std::cout << "[main] enter whisker main func." << std::endl;
#ifndef WIND32
    signal(SIGCHLD, SIG_DFL);   // 子进程退出时，默认处理（避免产生僵尸进程）
    signal(SIGPIPE, SIG_IGN);   // 忽略 SIGPIPE 信号（通常在管道破裂、socket 关闭时触发）
    signal(SIGINT, prog_exit);  // SIGINT (Ctrl+C) 和 SIGTERM (终止信号) 时调用
    signal(SIGTERM, prog_exit); // prog_exit 进行清理操作
#endif
    // 参数解析
    parse_arguments(argc, argv);

    // 读取配置
    ConfigParser       parser;
    struct WhispConfig Wconfig;
    if (parser.loadConfig(Wconfig)) {
        parser.printConfig(Wconfig);
    } else {
        std::cerr << "[main] load conifg.yml fail" << std::endl;
    }

    // 初始化日志
    std::string log_file;
    if (Wconfig.log_config.log_file_dir.size() == 0) {
        const char* default_path = "/var/log/whisplog";
        log_file += default_path;
    } else {
        log_file += Wconfig.log_config.log_file_dir;
    }

    std::cout << "[main] log file path is : " << log_file << std::endl;

    DIR* dp = opendir(log_file.c_str());
    if (dp == NULL) {
        if (mkdir(log_file.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
            std::cerr << "[main] create base dir error, " << log_file << ", errno: " << errno << strerror(errno);
            return 1;
        }
    }
    closedir(dp);
    if (Wconfig.log_config.log_file_name.size() == 0) {
        log_file += "default_log";
    } else {
        log_file += Wconfig.log_config.log_file_name;
    }
    std::cout << "[main] log file is : " << log_file << std::endl;
    if (WhispLog::get_instance().log_init(log_file.c_str())) {
        std::cout << "[main] log init return true" << std::endl;
    }
    LOG_INFO("[main] Init log module success");

    // 初始化数据库配置
    std::string db_server;
    int         db_port = 0;
    std::string db_user;
    std::string db_passpwd;
    std::string db_table;

    LOAD_CONFIG_VALUE(Wconfig.mysql_config.mysql_server_addr, db_server);
    db_port = Wconfig.mysql_config.mysql_server_port;
    LOAD_CONFIG_VALUE(Wconfig.mysql_config.user, db_user);
    LOAD_CONFIG_VALUE(Wconfig.mysql_config.password, db_passpwd);
    LOAD_CONFIG_VALUE(Wconfig.mysql_config.database, db_table);

    LOG_INFO(
        "[main] Load mysql config: addr = %s, port = %d, user = %s, "
        "password = %s, database = %s",
        db_server.c_str(), db_port, db_user.c_str(), db_passpwd.c_str(), db_table.c_str());

    // 创建工厂对象
    // WhispConcreteDbConnFactory factory(db_server, db_port, db_user, db_passpwd,
    // db_table, 5);
    // auto factory = std::make_unique<MySQLXFactory>(db_server, db_port, db_user, db_passpwd, db_table);

    // auto pool = std::make_shared<MySQLXPool>(std::move(factory), 5);
    if (!Singleton<CMysqlManager>::Instance().init(db_server.c_str(), db_user.c_str(), db_passpwd.c_str(),
                                                   db_table.c_str())) {
        LOG_FATAL("Init mysql failed, please check your database config..............");
    }

    if (!Singleton<UserManager>::Instance().init(db_server.c_str(), db_user.c_str(), db_passpwd.c_str(),
                                                 db_table.c_str())) {
        LOG_FATAL("Init UserManager failed, please check your database config..............");
    }

    // chat server
    const char* listenip   = Wconfig.client_config.client_listen_ip.c_str();
    short       listenport = Wconfig.client_config.client_listen_port;
    LOG_INFO("[main] Listen Server: %s:%u", listenip, listenport);
    Singleton<ChatServer>::Instance().init(listenip, listenport, &g_mainLoop);

    // monitor server
    const char* monitorip    = Wconfig.monitor_config.monitor_listen_ip.c_str();
    short       monitorport  = Wconfig.monitor_config.monitor_listen_port;
    const char* monitortoken = Wconfig.monitor_config.monitor_token.c_str();
    LOG_INFO("[main] Monitor Server: %s:%u", monitorip, monitorport);
    Singleton<MonitorServer>::Instance().init(monitorip, monitorport, &g_mainLoop, monitortoken);
    // http server
    const char* httpip   = Wconfig.http_config.http_listen_ip.c_str();
    short       httpport = Wconfig.http_config.http_listen_port;
    LOG_INFO("[main] Http Server: %s:%u", httpip, httpport);
    Singleton<HttpServer>::Instance().init(httpip, httpport, &g_mainLoop);

    // 析构
    WhispLog::get_instance().log_uninit();
    std::cout << "[main] log uninit return true" << std::endl;

    return 0;
}