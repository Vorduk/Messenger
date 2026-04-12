#pragma once
#include <string>
#include <chrono>

struct Message {
    std::string sender;
    std::string text;
    std::chrono::system_clock::time_point timestamp;

    Message() = default;
    Message(const std::string& sender, 
            const std::string& text, 
            std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now())
            : sender(sender), text(text), timestamp(timestamp) {
    }
};