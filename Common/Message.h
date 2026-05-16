#pragma once
#include <string>
#include <chrono>
#include "MessageStatus.h"

/**
 * @brief Represents a chat message.
 */
struct Message {
    std::string id;               // UUID
    std::string sender_id;        // User ID of sender
    std::string receiver_id;      // User ID of receiver
    std::string text;             // Message content
    std::chrono::system_clock::time_point timestamp;
    MessageStatus status = MessageStatus::Sending;

    Message() : timestamp(std::chrono::system_clock::now()) {}

    Message(const std::string& sender_id,
        const std::string& receiver_id,
        const std::string& text)
        : sender_id(sender_id)
        , receiver_id(receiver_id)
        , text(text)
        , timestamp(std::chrono::system_clock::now()) {
    }
};