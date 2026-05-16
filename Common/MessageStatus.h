#pragma once

/**
 * @brief Status of a sent message.
 */
enum class MessageStatus {
    Sending,      // Still being sent
    Sent,         // Delivered to server
    Delivered,    // Delivered to recipient
    Read,         // Read by recipient
    Failed        // Failed to send
};