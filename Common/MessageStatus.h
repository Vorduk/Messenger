#pragma once
#include <string>

/**
 * @brief Status of a sent message.
 */
enum class MessageStatus {
    Sending,      // Still being sent
    Sent,         // Delivered to server
    Delivered,    // Delivered to recipient
    Read,         // Read by recipient
    Failed,       // Failed to send
    Default,      // Default
};

inline std::string messageStatusToString(MessageStatus status) {
    switch (status) {
    case MessageStatus::Sending:   return "sending...";
    case MessageStatus::Sent:      return "sent";
    case MessageStatus::Delivered: return "delivered";
    case MessageStatus::Read:      return "read";
    case MessageStatus::Failed:    return "failed";
    default:                       return "";
    }
}