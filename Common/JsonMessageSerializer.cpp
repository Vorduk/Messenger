#include "JsonMessageSerializer.h"
#include <chrono>

std::string JsonMessageSerializer::serialize(const Message& msg) const {
    nlohmann::json j;
    j["sender"] = msg.sender;
    j["text"] = msg.text;
    j["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        msg.timestamp.time_since_epoch()).count();
    return j.dump();
}

Message JsonMessageSerializer::deserialize(const std::string& data) const {
    auto j = nlohmann::json::parse(data);
    Message msg;
    msg.sender = j.at("sender");
    msg.text = j.at("text");
    auto ms = j.at("timestamp").get<long long>();
    msg.timestamp = std::chrono::system_clock::time_point(
        std::chrono::milliseconds(ms));
    return msg;
}