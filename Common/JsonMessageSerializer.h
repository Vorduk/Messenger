#pragma once
#include "IMessageSerializer.h"
#include <json.hpp>

class JsonMessageSerializer : public IMessageSerializer {
public:
    std::string serialize(const Message& message) const override;
    Message deserialize(const std::string& data) const override;
};