#pragma once
#include <string>

class ISendMessageHandler {
public:
    virtual ~ISendMessageHandler() = default;
    virtual void onUserSendMessage(const std::string& sender,
        const std::string& receiver,
        const std::string& text) = 0;
};