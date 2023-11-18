#ifndef CHAT_FRIENDMODEL_H
#define CHAT_FRIENDMODEL_H

#include "friend.h"
#include "user.h"

#include <vector>

class FriendModel {
public:
    //添加好友关系
    bool insert(Friend aFriend);

    //返回用户好友列表
    std::vector<User> query(int userid);
};


#endif //CHAT_FRIENDMODEL_H
