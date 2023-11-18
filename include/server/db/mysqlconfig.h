#ifndef CHAT_MYSQLCONFIG_H
#define CHAT_MYSQLCONFIG_H


#include <string>
#include <unordered_map>
#include <fstream>

class MySQLConfig {
public:
    explicit MySQLConfig(const std::string& filename);

    std::string get(const std::string& section, const std::string& key);

private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_configMap;
};


#endif //CHAT_MYSQLCONFIG_H
