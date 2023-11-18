#include "connection.h"

#include <muduo/base/Logging.h>

Connection::Connection()
    : m_conn{ mysql_init(nullptr) } {}

Connection::~Connection() {
    if (m_conn != nullptr) {
        mysql_close(m_conn);
    }
}

bool Connection::connect(const std::string& ip, unsigned short port, const std::string& username,
                         const std::string& password,
                         const std::string& dbname) {
    MYSQL* p{ mysql_real_connect(m_conn, ip.c_str(), username.c_str(), password.c_str(),
                                 dbname.c_str(), port, nullptr, 0) };

    if (p != nullptr) {
        //C++默认编码为ASCII，防止中文乱码
//        mysql_query(m_conn, "set names utf-8");
    }

    return p != nullptr;
}

bool Connection::update(const std::string& sql) {
    if (mysql_query(m_conn, sql.c_str())) {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "更新失败！";
        LOG_ERROR << "MySQL Error: " << mysql_error(m_conn);
        return false;
    }

    return true;
}


MYSQL_RES* Connection::query(const std::string& sql) {
    if (mysql_query(m_conn, sql.data())) {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "查询失败！";
        LOG_ERROR << "MySQL Error: " << mysql_error(m_conn);
        return nullptr;
    }

    return mysql_use_result(m_conn);
}

void Connection::refreshAliveTime() {
    m_aliveTime = clock();
}

clock_t Connection::getAliveTime() const {
    return clock() - m_aliveTime;
}

MYSQL* Connection::getRawConn() {
    return m_conn;
}
