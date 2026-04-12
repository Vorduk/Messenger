#pragma once
#include <string>

class IMessageSender {
public:
    virtual ~IMessageSender() = default;
    virtual bool send(const std::string& serialized_message) = 0;
};