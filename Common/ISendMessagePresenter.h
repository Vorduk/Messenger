#pragma once
#include <string>

class ISendMessagePresenter {
public:
    virtual ~ISendMessagePresenter() = default;
    virtual void onMessageSent(const std::string& message_id, const std::string& confirmation) = 0;
    virtual void onError(const std::string& error) = 0;
};