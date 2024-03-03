#include "usermodel.h"

#include <chatservice.h>
#include <cstring>
#include <sstream>

#include "user.h"
#include "db/connectionpool.h"

// // 函数用于将十六进制字符串转换为二进制数据
// std::string hexStringToBinary(const std::string& hexString) {
//     std::string binaryString;
//
//     for (size_t i = 0; i < hexString.length(); i += 2) {
//         unsigned int byteValue;
//         std::istringstream(hexString.substr(i, 2)) >> std::hex >> byteValue;
//         binaryString.push_back(static_cast<char>(byteValue));
//     }
//
//     return binaryString;
// }
//
// // 函数用于将二进制数据解码为 Base64 字符串
// std::string binaryToBase64(const std::string& binaryData) {
//     // 你可以使用你之前实现的 Base64 编码函数，或者使用现有的库
//     // 这里假设有一个名为 base64_encode 的函数可以使用
//     // 替换为实际的 Base64 编码实现
//     return ChatService::base64_encode(binaryData);
// }
//
// // 函数用于将十六进制字符串（存储在 MySQL 中）转换为原始数据并解码为 Base64
// std::string convertHexFromMySQL(const std::string& hexString) {
//     // 将十六进制字符串转换为二进制数据
//     std::string binaryData = hexStringToBinary(hexString);
//
//     // 将二进制数据解码为 Base64 字符串
//     // std::string base64String = binaryToBase64(binaryData);
//
//     return binaryData;
// }

std::string convertToHex(const std::string& input) {
    std::string hexString;
    static const char hex_chars[] = "0123456789ABCDEF";

    for (char byte: input) {
        hexString.push_back(hex_chars[(byte & 0xF0) >> 4]);
        hexString.push_back(hex_chars[byte & 0x0F]);
    }

    return hexString;
}

bool UserModel::insert(User& user) {
    // //组装SQL
    // char sql[1024]{};
    // sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s')", user.getName().c_str(),
    //         user.getPassword().c_str(), user.getState().c_str());
    //
    // //从连接池获取MySQL连接
    // ConnectionPool* connectionPool{ ConnectionPool::getInstance() };
    // std::shared_ptr<Connection> pConn{ connectionPool->getConnection() };
    //
    // if (bool res = pConn->update(sql); res) {
    //     //获取插入成功的用户的主键
    //     user.setId(mysql_insert_id(pConn->getRawConn()));
    //     return true;
    // } else {
    //     return false;
    // }

    // 获取数据库连接
    ConnectionPool* connectionPool{ ConnectionPool::getInstance() };
    std::shared_ptr<Connection> pConn{ connectionPool->getConnection() };

    if (!pConn) {
        fprintf(stderr, "Failed to get database connection\n");
        return false;
    }

    // 将头像数据转换为十六进制字符串
    const std::string& headImageHex = convertToHex(user.getHeadImage());

    // 组装 SQL 语句
    std::string sql = "INSERT INTO user (name, password, state, head_image) VALUES ('" +
                      user.getName() + "', '" +
                      user.getPassword() + "', '" +
                      user.getState() + "', X'" +
                      headImageHex + "')";

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
    // sprintf(sql, "select id, name, password, state, head_image from user where id = %d", id);
    sprintf(sql, "SELECT id, name, password, state, TO_BASE64(head_image) FROM user WHERE id = %d", id);

    //从连接池获取MySQL连接
    ConnectionPool* connectionPool{ ConnectionPool::getInstance() };
    std::shared_ptr<Connection> pConn{ connectionPool->getConnection() };

    if (auto res = pConn->query(sql); res) {
        auto row{ mysql_fetch_row(res) };
        if (row) {
            // auto r4 = convertHexFromMySQL(row[4]);
            User user{ row[4], std::atoi(row[0]), row[1], row[2], row[3] };
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
