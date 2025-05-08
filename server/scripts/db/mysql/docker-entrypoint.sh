#!/bin/sh

# 如果数据目录为空，则初始化数据库
if [ -z "$(ls -A /var/lib/mysql)" ]; then
    mysql_install_db --user=mysql --datadir=/var/lib/mysql
    
    # 启动临时服务器设置root密码
    mysqld_safe --datadir=/var/lib/mysql --nowatch &
    sleep 5
    
    # 设置root密码和创建初始数据库
    mysql -uroot <<EOF
UPDATE mysql.user SET Password=PASSWORD('$MYSQL_ROOT_PASSWORD') WHERE User='root';
DELETE FROM mysql.user WHERE User='';
DELETE FROM mysql.user WHERE User='root' AND Host NOT IN ('localhost', '127.0.0.1', '::1');
DROP DATABASE IF EXISTS test;
DELETE FROM mysql.db WHERE Db='test' OR Db='test\\_%';
CREATE DATABASE IF NOT EXISTS $MYSQL_DATABASE;
GRANT ALL ON $MYSQL_DATABASE.* TO '$MYSQL_USER'@'%' IDENTIFIED BY '$MYSQL_PASSWORD';
FLUSH PRIVILEGES;
EOF
    
    # 关闭临时服务器
    mysqladmin -uroot -p$MYSQL_ROOT_PASSWORD shutdown
    sleep 5
fi

exec "$@"