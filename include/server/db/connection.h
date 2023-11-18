#ifndef CHAT_CONNECTION_H
#define CHAT_CONNECTION_H


#include "mysql/mysql.h"

#include <string>
#include <ctime>

class Connection {
public:
    Connection();
    ~Connection();

    bool connect(const std::string& ip, unsigned short port, const std::string& username, const std::string& password,
                 const std::string& dbname);
    bool update(const std::string& sql);
    MYSQL_RES* query(const std::string& sql);
    MYSQL* getRawConn();

    void refreshAliveTime();   //刷新连接的起始空闲时间
    clock_t getAliveTime() const;    //返回存活的时间

private:
    MYSQL* m_conn;
    clock_t m_aliveTime;    //记录进入空闲状态后的存活时间
};


#endif //CHAT_CONNECTION_H
