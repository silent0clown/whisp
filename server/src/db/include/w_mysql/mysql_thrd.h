#pragma once

#include <condition_variable>
#include <thread>

#include "db_mysql.h"
#include "w_mysql/mysql_task.h"
#include "w_mysql/task_list.h"

class CMysqlThrd {
   public:
    CMysqlThrd(void);
    ~CMysqlThrd(void);

    void Run();

    bool start(const std::string& host, const std::string& user, const std::string& pwd, const std::string& dbname);
    void stop();
    bool addTask(IMysqlTask* poTask)
    {
        return m_oTask.push(poTask);
    }

    IMysqlTask* getReplyTask(void)
    {
        return m_oReplyTask.pop();
    }

   protected:
    bool init();
    void mainLoop();
    void uninit();

   private:
    bool                         m_bTerminate;
    std::unique_ptr<std::thread> m_pThread;
    bool                         m_bStart;
    CDatabaseMysql*              m_poConn;
    CTaskList                    m_oTask;
    CTaskList                    m_oReplyTask;

    std::mutex              m_mutex;
    std::condition_variable m_cond;
};
