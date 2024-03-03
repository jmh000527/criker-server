#include <cstring>
#include "groupmodel.h"
#include "connectionpool.h"

bool GroupModel::createGroup(Group& group) {
    //组装SQL
    char sql[1024]{};
    sprintf(sql, "insert into all_group(groupname, groupdesc) values('%s', '%s')", group.getName().c_str(),
            group.getDesc().c_str());

    //从连接池获取MySQL连接
    ConnectionPool* connectionPool{ ConnectionPool::getInstance() };
    std::shared_ptr<Connection> pConn{ connectionPool->getConnection() };

    if (bool res = pConn->update(sql); res) {
        //获取插入成功的群组的主键
        group.setId(mysql_insert_id(pConn->getRawConn()));
        return true;
    } else {
        return false;
    }
}

bool GroupModel::addGroup(int userid, int groupid, const std::string& role) {
    //组装SQL
    char sql[1024]{};
    sprintf(sql, "insert into group_user values(%d, %d, '%s')", groupid, userid, role.c_str());

    //从连接池获取MySQL连接
    ConnectionPool* connectionPool{ ConnectionPool::getInstance() };
    std::shared_ptr<Connection> pConn{ connectionPool->getConnection() };

    if (bool res = pConn->update(sql); res) {
        return true;
    } else {
        return false;
    }
}

std::vector<Group> GroupModel::queryGroups(int userid) {
    //组装SQL
    char sql[1024]{};
    sprintf(sql,
            "select a.id,a.groupname,a.groupdesc from all_group a inner join group_user b on a.id = b.groupid where b.userid = %d",
            userid);

    //从连接池获取MySQL连接
    ConnectionPool* connectionPool{ ConnectionPool::getInstance() };
    std::shared_ptr<Connection> pConn{ connectionPool->getConnection() };

    std::vector<Group> groups{};

    if (auto res = pConn->query(sql); res) {
        //查出userid所在的所有群组信息
        while (auto row = mysql_fetch_row(res)) {
            groups.emplace_back(std::atoi(row[0]), row[1], row[2]);
        }

        mysql_free_result(res);
    }

    //查询用户所在群组的所有用户信息
    for (Group& group: groups) {
        memset(sql, 0, sizeof sql);
        sprintf(sql,
                "SELECT a.id, a.name, a.state, b.grouprole, TO_BASE64(a.head_image) FROM user a INNER JOIN group_user b ON b.userid = a.id WHERE b.groupid = %d",
                group.getId());

        if (auto res = pConn->query(sql); res) {
            //查出userid所有的群组信息
            while (auto row = mysql_fetch_row(res)) {
                // group.getUsers().emplace_back(std::stoi(row[0]), row[1], "", row[2], row[3]);
                group.setUser(GroupUser{ row[4], std::stoi(row[0]), row[1], "", row[2], row[3] });
            }

            mysql_free_result(res);
        }
    }

    return groups;
}

std::vector<int> GroupModel::queryGroupUsers(int userid, int groupid) {
    //组装SQL
    char sql[1024]{};
    sprintf(sql,
            "select userid from group_user where groupid = %d and userid != %d", groupid, userid);

    //从连接池获取MySQL连接
    ConnectionPool* connectionPool{ ConnectionPool::getInstance() };
    std::shared_ptr<Connection> pConn{ connectionPool->getConnection() };

    std::vector<int> groupUsers{};

    if (auto res = pConn->query(sql); res) {
        //查出userid所在的所有群组信息
        while (auto row = mysql_fetch_row(res)) {
            groupUsers.push_back(std::atoi(row[0]));
        }

        mysql_free_result(res);
    }

    return groupUsers;
}
