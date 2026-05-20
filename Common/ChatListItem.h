#pragma once
#include "User.h"
#include "MessageStatus.h"
#include <chrono>

/**
 * @brief Combines a contact user with the preview of the last message in the conversation.
 */
struct ChatListItem {
    User user;                                               ///< The contact user
    std::string last_message;                                ///< Text of the last message
    std::chrono::system_clock::time_point last_message_time = std::chrono::system_clock::time_point::min(); ///< Timestamp (UTC)
    MessageStatus last_message_status = MessageStatus::Sent; ///< Status of the last message
};