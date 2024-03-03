#include "friendmodel.h"
#include "connectionpool.h"

bool FriendModel::insert(Friend aFriend) {
    //组装SQL
    char sql[1024]{};
    sprintf(sql, "insert into friend(userid, friendid) values(%d, %d)", aFriend.getUserId(), aFriend.getFriendId());

    //从连接池获取MySQL连接
    ConnectionPool* connectionPool{ ConnectionPool::getInstance() };
    std::shared_ptr<Connection> pConn{ connectionPool->getConnection() };

    if (bool res = pConn->update(sql); res) {
        return true;
    } else {
        return false;
    }
}

std::vector<User> FriendModel::query(int userid) {
    //组装SQL
    char sql[1024]{};
    sprintf(sql,
            "select a.id,a.name,a.state,TO_BASE64(a.head_Image) from user a inner join friend b on b.friendid = a.id where b.userid = %d",
            userid);

    //从连接池获取MySQL连接
    ConnectionPool* connectionPool{ ConnectionPool::getInstance() };
    std::shared_ptr<Connection> pConn{ connectionPool->getConnection() };

    std::vector<User> users{};

    if (auto res = pConn->query(sql); res) {
        while (auto row = mysql_fetch_row(res)) {
            User user{ row[3], std::atoi(row[0]), row[1], "", row[2] };

            users.push_back(user);
        }

        mysql_free_result(res);
    }

    return users;
}
