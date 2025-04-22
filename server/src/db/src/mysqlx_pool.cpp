// MySQLXPool.cpp
// MySQLXPool.cpp
#include "mysqlx_pool.h"
#include <chrono>
#include <iostream>

MySQLXPool::MySQLXPool(std::unique_ptr<IConnectionFactory> factory, size_t pool_size)
    : factory_(std::move(factory)), pool_size_(pool_size)
{
    for (size_t i = 0; i < pool_size_; i++) {
        auto conn = factory_->create();
        if (conn == nullptr) {
            continue;
        }
        pool_.push(std::move(conn));
    }
}

std::shared_ptr<IConnection> MySQLXPool::getConnection(int timeout_ms)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (!cond_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                        [this] { return !pool_.empty() || active_count_ < pool_size_; })) {
        std::cerr << "[MySQLXPool] Timeout while waiting for connection" << std::endl;
        return nullptr;
    }

    std::unique_ptr<IConnection> conn;
    if (!pool_.empty()) {
        conn = std::move(pool_.front());
        pool_.pop();
    } else {
        conn = factory_->create();
    }

    ++active_count_;
    return std::shared_ptr<IConnection>(conn.release(), ConnectionDeleter(this));
}

void MySQLXPool::returnConnection(std::unique_ptr<IConnection> conn)
{
    std::lock_guard<std::mutex> lock(mutex_);
    --active_count_;

    if (validateConnection(conn.get())) {
        pool_.push(std::move(conn));
    }
    cond_.notify_one();
}

bool MySQLXPool::validateConnection(IConnection* conn) const
{
    return conn && conn->isAlive();
}

size_t MySQLXPool::availableCount() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return pool_.size();
}

void MySQLXPool::ConnectionDeleter::operator()(IConnection* conn) const
{
    pool_->returnConnection(std::unique_ptr<IConnection>(conn));
}

/*
// 业务服务类示例
class OrderService {
public:
    explicit OrderService(std::shared_ptr<IConnectionPool> pool)
        : pool_(std::move(pool)) {}

    void createOrder(const Order& order) {
        auto conn = pool_->getConnection();
        std::string sql = fmt::format(
            "INSERT INTO orders VALUES('{}', {}, {})",
            order.id, order.amount, order.user_id);

        if (!conn->execute(sql)) {
            throw std::runtime_error("Create order failed");
        }
    }

private:
    std::shared_ptr<IConnectionPool> pool_;
};

// 依赖装配
int main() {
    // 1. 创建工厂
    auto factory = std::make_unique<MySQLXFactory>(
        "localhost", 33060, "user", "password", "commerce_db");

    // 2. 创建连接池
    auto pool = std::make_shared<MySQLXPool>(
        std::move(factory), 10);  // 池大小10

    // 3. 注入到业务服务
    OrderService orderService(pool);

    // 使用服务
    orderService.createOrder({ "order123", 100, 42 });

    return 0;
}

*/