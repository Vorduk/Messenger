#pragma once
#include "SendMessageUseCase.h"

class SendMessageController {
public:
    SendMessageController(SendMessageUseCase& use_case);
    void onUserSendMessage(const std::string& sender, const std::string& reciever, const std::string& text);
private:
    SendMessageUseCase& m_use_case;
};