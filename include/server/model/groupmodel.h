#ifndef CHAT_GROUPMODEL_H
#define CHAT_GROUPMODEL_H

//维护群组信息的model类

#include "group.h"

class GroupModel {
public:
    //创建群组
    bool createGroup(Group& group);

    //加入群组
    bool addGroup(int userid, int groupid, const std::string& role);

    //查询用户所在群组信息
    std::vector<Group> queryGroups(int userid);

    //根据指定的groupid查询群组用户id列表，除userid自己，用于给群组其他成员发消息
    std::vector<int> queryGroupUsers(int userid, int groupid);
};


#endif //CHAT_GROUPMODEL_H
