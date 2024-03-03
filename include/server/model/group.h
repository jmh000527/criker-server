#ifndef CHAT_GROUP_H
#define CHAT_GROUP_H

//all_group表的ORM类

#include "groupuser.h"

#include <string>
#include <vector>

class Group {
public:
    explicit Group(int id = -1, std::string name = "", std::string desc = "");

    int getId() const;
    std::string getName() const;
    std::string getDesc() const;

    void setId(int id);
    void setName(const std::string& name);
    void setDesc(const std::string& desc);

    std::vector<GroupUser> getUsers() const;

    void setUser(const GroupUser& group_user);
    void setUser(GroupUser&& group_user);

    void setUsers(std::vector<GroupUser> groupUsers);

private:
    int m_id;
    std::string m_name;
    std::string m_desc;
    std::vector<GroupUser> m_groupusers;
};


#endif //CHAT_GROUP_H
