// MySQLXFactory.h
#include "iconnection.h"
// #include "mysqlx_connection.h"

class MySQLXFactory : public IConnectionFactory {
   public:
    MySQLXFactory(const std::string& host, int port, const std::string& user, const std::string& password,
                  const std::string& database);

    std::unique_ptr<IConnection> create() override;

   private:
    const std::string host_;
    const int         port_;
    const std::string user_;
    const std::string password_;
    const std::string database_;
};