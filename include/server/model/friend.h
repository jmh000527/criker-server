#ifndef CHAT_FRIEND_H
#define CHAT_FRIEND_H

//friend表的ORM类

class Friend {
public:
    Friend(int userid, int friendid);

    int getUserId() const;
    int getFriendId() const;

    void setUserId(int userid);
    void setFriendId(int friendid);

private:
    int m_userid;
    int m_friendid;

};


#endif //CHAT_FRIEND_H
