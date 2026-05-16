#include "JsonMessageSerializer.h"
#include <chrono>

std::string JsonMessageSerializer::serialize(const Message& message) const {
    nlohmann::json j;
    j["id"] = message.id;
    j["sender_id"] = message.sender_id;
    j["receiver_id"] = message.receiver_id;
    j["text"] = message.text;
    long long ts = std::chrono::duration_cast<std::chrono::milliseconds>(
        message.timestamp.time_since_epoch()).count();
    j["timestamp"] = ts;
    j["status"] = static_cast<int>(message.status);
    return j.dump();
}

Message JsonMessageSerializer::deserialize(const std::string& data) const {
    nlohmann::json j = nlohmann::json::parse(data);
    Message message;
    message.id = j.at("id").get<std::string>();
    message.sender_id = j.at("sender_id").get<std::string>();
    message.receiver_id = j.at("receiver_id").get<std::string>();
    message.text = j.at("text").get<std::string>();
    long long ms = j.at("timestamp").get<long long>();
    message.timestamp = std::chrono::system_clock::time_point(
        std::chrono::milliseconds(ms));
    message.status = static_cast<MessageStatus>(j.at("status").get<int>());
    return message;
}