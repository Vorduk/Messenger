#pragma once
#include "Message.h"
#include <string>

class IMessageSerializer {
public:
    virtual ~IMessageSerializer() = default;
    virtual std::string serialize(const Message& message) const = 0;
    virtual Message deserialize(const std::string& data) const = 0;
};