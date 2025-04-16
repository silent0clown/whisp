#pragma once
#include "iconnection.h"
#include <openssl/ssl.h>
#include <mysqlx/xdevapi.h>
// #include "../../../third_party/linux/x86_64/mysqlconn/include/mysqlx/xdevapi.h"
// #include "mysqlx/xdevapi.h"

class MySQLXConnection : public IConnection {
public:
    explicit MySQLXConnection(mysqlx::Session&& session);
    ~MySQLXConnection() override;
    
    bool execute(const std::string& sql) override;
    bool isAlive() override;

private:
    mysqlx::Session session_;
};