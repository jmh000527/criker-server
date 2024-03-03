#include "groupuser.h"

#include <utility>

GroupUser::GroupUser(std::string base64String, int id, std::string name, std::string password, std::string state,
                     std::string role)
    : User(std::move(base64String), id, std::move(name), std::move(password), std::move(state)),
      m_role{ std::move(role) } {}

std::string GroupUser::getRole() const {
    return m_role;
}

void GroupUser::setRole(const std::string& role) {
    m_role = role;
}
