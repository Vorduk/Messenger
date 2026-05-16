#pragma once
#include "SendMessageUseCase.h"
#include "ISendMessageHandler.h"

class SendMessageController : public ISendMessageHandler {
public:
    explicit SendMessageController(SendMessageUseCase& use_case);
    void onUserSendMessage(const std::string& sender,
        const std::string& receiver,
        const std::string& text) override;
private:
    SendMessageUseCase& m_use_case;
};