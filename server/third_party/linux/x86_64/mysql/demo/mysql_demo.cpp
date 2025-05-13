#include <mysql/mysql.h>
#include <iostream>

int main()
{
    MYSQL* conn = mysql_init(NULL);
    if (!conn) {
        std::cerr << "mysql_init() failed\n";
        return 1;
    }

    // 连接数据库（修改用户名、密码、主机名、端口和数据库名）
    if (!mysql_real_connect(conn, "127.0.0.1", "chat", "123456", nullptr, 3306, NULL, 0)) {
        std::cerr << "mysql_real_connect() failed: " << mysql_error(conn) << "\n";
        mysql_close(conn);
        return 1;
    }
    else {
        std::cerr << 
    }

    if (mysql_query(conn, "SHOW DATABASES")) {
        std::cerr << "SHOW DATABASES failed: " << mysql_error(conn) << "\n";
        mysql_close(conn);
        return 1;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        std::cerr << "mysql_store_result() failed: " << mysql_error(conn) << "\n";
        mysql_close(conn);
        return 1;
    }

    std::cout << "Databases:\n";
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        std::cout << " - " << row[0] << "\n";
    }

    mysql_free_result(result);
    mysql_close(conn);
    return 0;
}
