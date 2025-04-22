// MySQLXFactory.cpp
#include "mysqlx_factory.h"
#include "log.h"
#include "mysqlx_connection.h"

MySQLXFactory::MySQLXFactory(const std::string& host, int port, const std::string& user, const std::string& password,
                             const std::string& database)
    : host_(host), port_(port), user_(user), password_(password), database_(database)
{
}

std::unique_ptr<IConnection> MySQLXFactory::create()
{
    try {
        mysqlx::Session session(host_, port_, user_, password_);
        session.sql("USE " + database_).execute();
        LOG_INFO("[mysql] create conn to %s:%d success", host_.c_str(), port_);
        return std::make_unique<MySQLXConnection>(std::move(session));
    } catch (const mysqlx::Error& e) {
        // LOG_ERROR("Connection failed: " + std::string(e.what()).c_str());
        LOG_ERROR("Connection failed: %s", e.what()); // 使用流式输出
        return nullptr;
    }
}