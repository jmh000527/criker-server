#ifndef CHAT_UTILS_H
#define CHAT_UTILS_H

#include <iostream>

#include "json.hpp"

// 通用函数模板
template<typename T>
T getValueFromJson(const nlohmann::json& json, const std::string& key) {
    // 如果是对象，直接获取值
    if (json.is_object()) {
        return json.value(key, T{});
    }
        // 如果是数组，获取第一个元素的值
    else if (json.is_array() && !json.empty()) {
        return json[0].value(key, T{});
    }
        // 其他情况返回默认值
    else {
        return T{};
    }
}

#endif //CHAT_UTILS_H
