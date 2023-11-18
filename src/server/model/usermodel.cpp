#include "usermodel.h"
#include "user.h"
#include "db/connectionpool.h"

bool UserModel::insert(User& user) {
    //组装SQL
    char sql[1024]{};
    sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s')", user.getName().c_str(),
            user.getPassword().c_str(), user.getState().c_str());

    //从连接池获取MySQL连接
    ConnectionPool* connectionPool{ ConnectionPool::getInstance() };
    std::shared_ptr<Connection> pConn{ connectionPool->getConnection() };

    if (bool res = pConn->update(sql); res) {
        //获取插入成功的用户的主键
        user.setId(mysql_insert_id(pConn->getRawConn()));
        return true;
    } else {
        return false;
    }
}

User UserModel::query(int id) {
    //组装SQL
    char sql[1024]{};
    sprintf(sql, "select * from user where id = %d", id);

    //从连接池获取MySQL连接
    ConnectionPool* connectionPool{ ConnectionPool::getInstance() };
    std::shared_ptr<Connection> pConn{ connectionPool->getConnection() };

    if (auto res = pConn->query(sql); res) {
        auto row{ mysql_fetch_row(res) };
        if (row) {
            User user{ std::atoi(row[0]), row[1], row[2], row[3] };
            mysql_free_result(res);

            return user;
        }
    }

    return User{};
}

bool UserModel::updateState(const User& user) {
    //组装SQL
    char sql[1024]{};
    sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(), user.getId());

    //从连接池获取MySQL连接
    ConnectionPool* connectionPool{ ConnectionPool::getInstance() };
    std::shared_ptr<Connection> pConn{ connectionPool->getConnection() };

    if (bool res = pConn->update(sql); res) {
        return true;
    } else {
        return false;
    }
}

bool UserModel::resetState() {
    //组装SQL
    char sql[1024]{ "update user set state = 'offline' where state = 'online'" };

    //从连接池获取MySQL连接
    ConnectionPool* connectionPool{ ConnectionPool::getInstance() };
    std::shared_ptr<Connection> pConn{ connectionPool->getConnection() };

    if (bool res = pConn->update(sql); res) {
        return true;
    } else {
        return false;
    }
}
