#include "mysqlconfig.h"

MySQLConfig::MySQLConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Failed to open configuration file: " + filename);
    }

    std::string section;
    std::string line;
    while (std::getline(file, line)) {
        // 处理行中的 '\r'，如果存在的话，用于处理windows的“\r\n”换行
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();  // 移除行尾的 '\r'
        }

        if (!line.empty() && line[0] == '[' && line.back() == ']') {
            section = line.substr(1, line.size() - 2);
        } else if (!section.empty()) {
            size_t equalsPos = line.find('=');
            if (equalsPos != std::string::npos) {
                std::string key = line.substr(0, equalsPos - 1);
                std::string value = line.substr(equalsPos + 2);
                m_configMap[section][key] = value;
            }
        }
    }
}

std::string MySQLConfig::get(const std::string& section, const std::string& key) {
    if (m_configMap.find(section) != m_configMap.end() &&
        m_configMap[section].find(key) != m_configMap[section].end()) {
        return m_configMap[section][key];
    }

    throw std::runtime_error("Key not found in configuration: [" + section + "] " + key);
}