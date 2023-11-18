#ifndef CHAT_USERMODEL_H
#define CHAT_USERMODEL_H

//user表的数据操作类

#include "user.h"

class UserModel {
public:
    //user表的增加方法
    bool insert(User& user);

    //根据用户ID查询用户信息
    User query(int id);

    //更新用户状态信息
    bool updateState(const User& user);

    //重置用户的state信息
    bool resetState();
};


#endif //CHAT_USERMODEL_H
