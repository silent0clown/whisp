// mysql_conn.h
#pragma once
#include <mysqlx/xdevapi.h>
#include <memory>

class IDbConn {
public:
    virtual ~IDbConn() = default;
    virtual mysqlx::Session& getSession() = 0;
    virtual void release() = 0;
};

class IDbConnPool {
public:
    virtual ~IDbConnPool() = default;
    virtual std::unique_ptr<IDbConn> getConn() = 0;
    virtual size_t availableCount() const = 0;
};

// class IDbConn {
// public:
//     virtual ~IDbPool() = default;
//     virtual mysqlx::Session& getSession() = 0;
//     virtual void release() = 0;
// };