// MySQLXPool.h
#pragma once

#include "iconnection.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

class MySQLXPool : public IConnectionPool {
public:
    MySQLXPool(std::unique_ptr<IConnectionFactory> factory, size_t pool_size);
    ~MySQLXPool() override = default;

    std::shared_ptr<IConnection> getConnection(int timeout_ms) override;
    size_t availableCount() const override;

private:
    struct ConnectionDeleter {
        explicit ConnectionDeleter(MySQLXPool* pool) : pool_(pool) {}
        void operator()(IConnection* conn) const;
    private:
        MySQLXPool* pool_;
    };

    void returnConnection(std::unique_ptr<IConnection> conn);
    bool validateConnection(IConnection* conn) const;

    std::unique_ptr<IConnectionFactory> factory_;
    std::queue<std::unique_ptr<IConnection>> pool_;
    mutable std::mutex mutex_;
    std::condition_variable cond_;
    const size_t pool_size_;
    size_t active_count_ = 0;
};