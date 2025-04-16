// iconnection.h
#pragma once
#include <memory>
#include <string>

class IConnection {
public:
    virtual ~IConnection() = default;
    virtual bool execute(const std::string& sql) = 0;
    virtual bool isAlive() = 0;
};

class IConnectionFactory {
public:
    virtual ~IConnectionFactory() = default;
    virtual std::unique_ptr<IConnection> create() = 0;
};

class IConnectionPool {
public:
    virtual ~IConnectionPool() = default;
    virtual std::shared_ptr<IConnection> getConnection(int timeout_ms = 5000) = 0;
    virtual size_t availableCount() const = 0;
};