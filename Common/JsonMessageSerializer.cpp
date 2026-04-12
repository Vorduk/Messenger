#include "JsonMessageSerializer.h"
#include <chrono>

std::string JsonMessageSerializer::serialize(const Message& message) const {
    nlohmann::json j;
    j["sender"] = message.sender;
    j["text"] = message.text;
    j["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        message.timestamp.time_since_epoch()).count();
    return j.dump();
}

Message JsonMessageSerializer::deserialize(const std::string& data) const {
    auto j = nlohmann::json::parse(data);
    Message message;
    message.sender = j.at("sender");
    message.text = j.at("text");
    auto ms = j.at("timestamp").get<long long>();
    message.timestamp = std::chrono::system_clock::time_point(
        std::chrono::milliseconds(ms));
    return message;
}