#pragma once

#include <stdint.h>
#include "w_mysql/mysql_task.h"

#define MAX_TASK_NUM 1000

class CTaskList {
   public:
    CTaskList();
    ~CTaskList(void);

    bool        push(IMysqlTask* poTask); // �߼��߳��޸�
    IMysqlTask* pop();                    // ���ݿ��߳��޸�

   private:
    uint16_t    m_uReadIndex;  // ���ݿ��߳��޸�
    uint16_t    m_uWriteIndex; // �߼��߳��޸�
    IMysqlTask* m_pTaskNode[MAX_TASK_NUM];
};
