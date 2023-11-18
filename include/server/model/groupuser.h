#ifndef CHAT_GROUPUSER_H
#define CHAT_GROUPUSER_H

#include "user.h"

#include <string>

class GroupUser : public User {
public:
    explicit GroupUser(int id, std::string name, std::string password, std::string state, std::string role);
    
    std::string getRole() const;
    void setRole(const std::string& role);

private:
    std::string m_role;

};


#endif //CHAT_GROUPUSER_H
