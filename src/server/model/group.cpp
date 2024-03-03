#include "group.h"

#include <utility>

Group::Group(int id, std::string name, std::string desc)
    : m_id{ id }, m_name{ std::move(name) }, m_desc{ std::move(desc) } {}

int Group::getId() const {
    return m_id;
}

std::string Group::getName() const {
    return m_name;
}

std::string Group::getDesc() const {
    return m_desc;
}

void Group::setId(int id) {
    m_id = id;
}

void Group::setName(const std::string& name) {
    m_name = name;
}

void Group::setDesc(const std::string& desc) {
    m_desc = desc;
}

std::vector<GroupUser> Group::getUsers() const {
    return m_groupusers;
}

void Group::setUser(const GroupUser& group_user) {
    m_groupusers.push_back(group_user);
}

// 右值版本
void Group::setUser(GroupUser&& group_user) {
    m_groupusers.push_back(std::move(group_user));
}

