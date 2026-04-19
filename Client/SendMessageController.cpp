#include "SendMessageController.h"

SendMessageController::SendMessageController(SendMessage& use_case)
    : m_use_case(use_case) {
}

void SendMessageController::onUserSendMessage(const std::string& sender, const std::string& reciever, const std::string& text) {
    m_use_case.execute(sender, reciever, text);
}