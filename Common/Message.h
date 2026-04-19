#pragma once
#include <string>
#include <chrono>

/**
 * @struct Message
 * 
 * @brief
 * Includes information:
 * - message sender
 * - message reciever
 * - text of message
 * - timestamp
 * Used in both client and server.
 * Serialized to Json via JsonMessageSerializer.
 */
struct Message {
    std::string sender;
    std::string receiver;
    std::string text;
    std::chrono::system_clock::time_point timestamp;

    Message() = default;
    Message(const std::string& sender,
            const std::string& receiver,
            const std::string& text, 
            std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now())
            : sender(sender), receiver(receiver), text(text), timestamp(timestamp) {
    }
};