#ifndef CHAT_OFFLINEMESSAGE_H
#define CHAT_OFFLINEMESSAGE_H

//offline_message表的ORM类

#include <string>

class OfflineMessage {
public:
    explicit OfflineMessage(int id = -1, std::string message = "");

    [[nodiscard]] int getId() const;
    [[nodiscard]] std::string getMessage() const;

    void setId(int id);
    void setMessage(const std::string& message);

private:
    int m_id;
    std::string m_message;

};


#endif //CHAT_OFFLINEMESSAGE_H
