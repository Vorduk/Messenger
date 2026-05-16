#include "JsonMessageSerializer.h"
#include <chrono>

std::string JsonMessageSerializer::serialize(const Message& message) const {
    nlohmann::json j;
    j["sender_id"] = message.sender_id;
    j["receiver_id"] = message.receiver_id;
    j["text"] = message.text;
    j["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        message.timestamp.time_since_epoch()).count();
    return j.dump();
}

Message JsonMessageSerializer::deserialize(const std::string& data) const {
    auto j = nlohmann::json::parse(data);
    Message message;
    message.sender_id = j.at("sender_id");
    message.receiver_id = j.at("receiver_id");
    message.text = j.at("text");
    auto ms = j.at("timestamp").get<long long>();
    message.timestamp = std::chrono::system_clock::time_point(
        std::chrono::milliseconds(ms));
    return message;
}