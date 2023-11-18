#include "offlinemessagemodel.h"

#include <connectionpool.h>

bool OfflineMessageModel::insert(const OfflineMessage& offlineMessage) {
    //组装SQL
    char sql[1024]{};
    sprintf(sql, "insert into offline_message (userid, message) values(%d, '%s')", offlineMessage.getId(),
            offlineMessage.getMessage().c_str());

    //从连接池获取MySQL连接
    ConnectionPool* connectionPool{ ConnectionPool::getInstance() };
    std::shared_ptr<Connection> pConn{ connectionPool->getConnection() };

    if (bool res = pConn->update(sql); res) {
        return true;
    } else {
        return false;
    }
}

bool OfflineMessageModel::remove(int userid) {
    //组装SQL
    char sql[1024]{};
    sprintf(sql, "delete from offline_message where userid = %d", userid);

    //从连接池获取MySQL连接
    ConnectionPool* connectionPool{ ConnectionPool::getInstance() };
    std::shared_ptr<Connection> pConn{ connectionPool->getConnection() };

    if (bool res = pConn->update(sql); res) {
        return true;
    } else {
        return false;
    }
}

std::vector<OfflineMessage> OfflineMessageModel::query(int userid) {
    //组装SQL
    char sql[1024]{};
    sprintf(sql, "select * from offline_message where userid = %d", userid);

    //从连接池获取MySQL连接
    ConnectionPool* connectionPool{ ConnectionPool::getInstance() };
    std::shared_ptr<Connection> pConn{ connectionPool->getConnection() };

    std::vector<OfflineMessage> msgs{};

    if (auto res = pConn->query(sql); res) {
        while (auto row = mysql_fetch_row(res)) {
            msgs.emplace_back(std::atoi(row[0]), row[1]);
        }

        mysql_free_result(res);
    }

    return msgs;
}
