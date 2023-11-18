#include "offlinemessage.h"

OfflineMessage::OfflineMessage(int id, std::string message)
    : m_id{ id }, m_message{ std::move(message) } {}

int OfflineMessage::getId() const {
    return m_id;
}

std::string OfflineMessage::getMessage() const {
    return m_message;
}

void OfflineMessage::setId(int id) {
    m_id = id;
}

void OfflineMessage::setMessage(const std::string& message) {
    m_message = message;
}