/**
 * ��ʱͨѶ��ҵ���߼���ͳһ�������BusinessLogic.h
 * zhangyl 2018.05.16
 */
#pragma once

#include <memory>
#include "tcp_connection.h"

using namespace network;

class BussinessLogic final {
   private:
    BussinessLogic()  = delete;
    ~BussinessLogic() = delete;

    BussinessLogic(const BussinessLogic& rhs)            = delete;
    BussinessLogic& operator=(const BussinessLogic& rhs) = delete;

   public:
    static void registerUser(const std::string& data, const std::shared_ptr<TcpConnection>& conn, bool keepalive,
                             std::string& retData);
};
