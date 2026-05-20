#pragma once
#include <string>
#include <chrono>

/**
 * @brief Represents a user in the messenger system.
 */
struct User {
    std::string id;             // UUID or auto-incremented ID
    std::string username;       // Unique, unchanging identifier
    std::string display_name;   // Changeable display name
    std::string bio;            // Optional profile description
    std::string avatar_path;    // Path to avatar image (placeholder for now)
    std::string birthday;       // ISO 8601 date string (YYYY-MM-DD)
    bool is_online = false;     // Current online status
    bool show_online = true;    // Privacy setting: show/hide online status
    bool show_typing = true;    // Privacy setting: show/hide typing indicator
    std::string last_message;   // Last message text
    std::chrono::system_clock::time_point last_message_time; // Last message time (UTC)

    User() : last_message_time(std::chrono::system_clock::time_point::min()) {}

    User(const std::string& username, const std::string& display_name)
        : username(username)
        , display_name(display_name)
        , last_message_time(std::chrono::system_clock::time_point::min()) {
    }
};