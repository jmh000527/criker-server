#ifndef CHAT_OFFLINEMESSAGEMODEL_H
#define CHAT_OFFLINEMESSAGEMODEL_H

//offline_message表的数据操作类

#include "offlinemessage.h"
#include <vector>

class OfflineMessageModel {
public:
    //存储用户的离线消息
    bool insert(const OfflineMessage& offlineMessage);

    //删除用户的离线消息
    bool remove(int userid);

    //查询用户的离线消息
    std::vector<OfflineMessage> query(int userid);
};


#endif //CHAT_OFFLINEMESSAGEMODEL_H
