// MySQLXConnectionPool.cpp
#include "idb_pool.h"
#include <mysqlx/xdevapi.h>
// #include <queue>
// #include <mutex>
// #include <condition_variable>

class MysqlDbPoolImpl : public IDbConn{
public:
    explicit MysqlDbPoolImpl(const std::string& uri) {
        pool = std::make_unique<mysqlx::SessionPool>(uri, 4, 10);
    }

    mysqlx::Session getSession() override {
        return pool->getSession();
    }

private:
    std::unique_ptr<mysqlx::SessionPool> pool;
};

// class MySQLXConnection : public IConnection {
// public:
//     explicit MySQLXConnection(mysqlx::Session&& session) 
//         : session_(std::move(session)), valid_(true) {}
    
//     mysqlx::Session& getSession() override {
//         if (!valid_) throw std::runtime_error("Connection invalid");
//         return session_;
//     }
    
//     void release() override {
//         valid_ = false; // 标记为不可用
//     }
    
//     ~MySQLXConnection() override {
//         if (valid_) {
//             try { session_.close(); } 
//             catch (...) {} // 确保析构不抛异常
//         }
//     }

// private:
//     mysqlx::Session session_;
//     bool valid_;
// };

// class MySQLXConnectionPool : public IConnectionPool {
// public:
//     MySQLXConnectionPool(const std::string& uri, 
//                         size_t max_connections = 10,
//                         std::chrono::seconds timeout = std::chrono::seconds(30))
//         : uri_(uri), max_connections_(max_connections), timeout_(timeout) {}
    
//     void initialize() {
//         for (size_t i = 0; i < max_connections_ / 2; ++i) {
//             pool_.push(createNewConnection());
//         }
//     }
    
//     std::unique_ptr<IConnection> getConnection() override {
//         std::unique_lock<std::mutex> lock(mutex_);
        
//         if (!cond_.wait_for(lock, timeout_, [this] { 
//             return !pool_.empty() || active_connections_ < max_connections_; 
//         })) {
//             throw std::runtime_error("Get connection timeout");
//         }
        
//         if (pool_.empty()) {
//             ++active_connections_;
//             return createNewConnection();
//         }
        
//         auto conn = std::move(pool_.front());
//         pool_.pop();
//         ++active_connections_;
        
//         // 返回带自定义删除器的unique_ptr
//         return std::unique_ptr<IConnection>(
//             conn.release(),
//             [this](IConnection* raw_conn) {
//                 returnConnection(std::unique_ptr<IConnection>(raw_conn));
//             });
//     }
    
//     size_t availableCount() const override {
//         std::lock_guard<std::mutex> lock(mutex_);
//         return pool_.size();
//     }

//     ~MySQLXConnectionPool() override {
//         std::lock_guard<std::mutex> lock(mutex_);
//         while (!pool_.empty()) {
//             pool_.pop(); // unique_ptr自动释放连接
//         }
//     }

// private:
//     std::unique_ptr<IConnection> createNewConnection() {
//         try {
//             return std::make_unique<MySQLXConnection>(
//                 mysqlx::Session(uri_));
//         } catch (const mysqlx::Error& e) {
//             throw std::runtime_error("MySQL connection failed: " + std::string(e.what()));
//         }
//     }
    
//     void returnConnection(std::unique_ptr<IConnection> conn) {
//         std::lock_guard<std::mutex> lock(mutex_);
//         --active_connections_;
//         auto mx_conn = dynamic_cast<MySQLXConnection*>(conn.get());
//         if (mx_conn) {
//             mx_conn->release();
//             pool_.push(std::move(conn));
//             cond_.notify_one();
//         }
//     }
    
//     std::queue<std::unique_ptr<IConnection>> pool_;
//     mutable std::mutex mutex_;
//     std::condition_variable cond_;
//     size_t active_connections_ = 0;
//     const size_t max_connections_;
//     const std::chrono::seconds timeout_;
//     std::string uri_;
// };