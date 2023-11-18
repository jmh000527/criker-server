#include "friend.h"

Friend::Friend(int userid, int friendid)
    : m_userid{ userid }, m_friendid{ friendid } {}

int Friend::getUserId() const {
    return m_userid;
}

int Friend::getFriendId() const {
    return m_friendid;
}

void Friend::setUserId(int userid) {
    m_userid = userid;
}

void Friend::setFriendId(int friendid) {
    m_friendid = friendid;
}
