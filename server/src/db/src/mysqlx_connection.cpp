#include "mysqlx_connection.h"
#include <iostream>

MySQLXConnection::MySQLXConnection(mysqlx::Session&& session) : session_(std::move(session))
{
    // 可以在这里打印调试信息
    std::cout << "[MySQLXConnection] Session initialized." << std::endl;
}

MySQLXConnection::~MySQLXConnection()
{
    try {
        if (isAlive()) {
            session_.close();
        }
    } catch (const mysqlx::Error& err) {
        std::cerr << "[MySQLXConnection] Error closing session: " << err.what() << std::endl;
    }
}

bool MySQLXConnection::execute(const std::string& sql)
{
    try {
        auto result = session_.sql(sql).execute();
        return true;
    } catch (const mysqlx::Error& err) {
        std::cerr << "[MySQLXConnection] SQL execution failed: " << err.what() << std::endl;
        return false;
    }
}

bool MySQLXConnection::isAlive()
{
    try {
        // 尝试获取默认数据库的 schema，检查连接是否活跃
        session_.sql("SELECT 1").execute();
        return true;
    } catch (const mysqlx::Error& err) {
        std::cerr << "[MySQLXConnection] Connection not alive: " << err.what() << std::endl;
        return false;
    }
}
