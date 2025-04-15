// DbPoolFactory.h
#pragma once
#include "IDbPool.h"
#include "MysqlDbPoolImpl.h"
#include "config_parse.h"  // 假设有配置解析模块

class DbPoolFactory {
public:
    static std::shared_ptr<IDbPool> create() {
        auto uri = ConfigParse::get("mysql_uri");  // 从配置中获取连接字符串
        return std::make_shared<MysqlDbPoolImpl>(uri);
    }
};