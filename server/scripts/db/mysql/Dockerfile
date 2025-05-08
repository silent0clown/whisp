FROM alpine:3.16

# 使用阿里云镜像源加速
RUN sed -i 's/dl-cdn.alpinelinux.org/mirrors.aliyun.com/g' /etc/apk/repositories

# 安装MySQL (MariaDB) 和必要的依赖
RUN apk add --no-cache \
    mariadb \
    mariadb-client \
    && mkdir -p /run/mysqld \
    && chown -R mysql:mysql /run/mysqld

# 配置MySQL
COPY my.cnf /etc/mysql/my.cnf
COPY init.sql /docker-entrypoint-initdb.d/

# 初始化脚本
COPY docker-entrypoint.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/docker-entrypoint.sh

# 数据卷
VOLUME /var/lib/mysql

# 暴露MySQL默认端口
EXPOSE 3306

# 设置环境变量
ENV MYSQL_ROOT_PASSWORD=changeme
ENV MYSQL_DATABASE=defaultdb
ENV MYSQL_USER=defaultuser
ENV MYSQL_PASSWORD=defaultpass

# 入口点
ENTRYPOINT ["docker-entrypoint.sh"]

# 启动命令
CMD ["mysqld", "--user=mysql"]


#### build docker images
# docker build -t minimal-mysql .

#### run image
# docker run -d \
#   -p 3306:3306 \
#   -e MYSQL_ROOT_PASSWORD=my-secret-pw \
#   -e MYSQL_DATABASE=myapp \
#   -e MYSQL_USER=myuser \
#   -e MYSQL_PASSWORD=mypassword \
#   -v mysql_data:/var/lib/mysql \
#   --name mysql-container \
#   minimal-mysql