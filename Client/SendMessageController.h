#pragma once
#include "SendMessage.h"

class SendMessageController {
public:
    SendMessageController(SendMessage& use_case);
    void onUserSendMessage(const std::string& sender, const std::string& text);
private:
    SendMessage& m_use_case;
};