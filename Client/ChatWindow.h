#pragma once
#include "DockableWindow.h"
#include "ISendMessageHandler.h"
#include "GetMessagesUseCase.h"
#include "StyleManager.h"
#include "Message.h"
#include <string>
#include <vector>
#include <mutex>
#include <chrono>

/**
 * @brief ImGui window displaying a chat conversation with a selected user.
 *
 * All server calls are asynchronous and non-blocking.
 */
class ChatWindow : public DockableWindow {
public:
    /**
     * @brief Constructs the chat window.
     * @param current_style Style settings for rendering (owned by StyleManager).
     */
    ChatWindow(const ChatWindowStyle& current_style);

    /** 
     * @return Window title used by the docking system. 
     */
    const char* getName() const override;

    /** 
     * @brief Renders the chat UI. Called every frame. 
     */
    void render() override;

    /**
     * @brief Sets the message send handler.
     * @param handler Pointer to the handler invoked when the user sends a message.
     */
    void SetHandler(ISendMessageHandler* handler);

    /**
     * @brief Sets the current conversation participants and clears history.
     * @param sender_id ID of the current (local) user.
     * @param receiver_id ID of the remote user (chat partner).
     * @param receiver_name Display name of the remote user.
     */
    void SetUsers(const std::string& sender_id,
        const std::string& receiver_id,
        const std::string& receiver_name);

    /**
     * @brief Adds a single message to the chat history.
     * @param sender ID of the message sender.
     * @param text Message content.
     * @param is_local If true, the message is displayed as "own" (right-aligned).
     *                 If false, determined automatically from sender_id.
     * @param timestamp Message timestamp (default: now).
     * @param msg_id Unique message identifier (auto-generated if empty).
     * @param status Delivery status (default: Sending).
     */
    void AddMessage(const std::string& sender,
        const std::string& text,
        bool is_local = false,
        const std::chrono::system_clock::time_point& timestamp = std::chrono::system_clock::now(),
        const std::string& msg_id = "",
        MessageStatus status = MessageStatus::Sending);

    /**
     * @brief Replaces the entire chat history with messages from the server.
     * @param messages Vector of Message objects (ordered newest-first from server,
     *                 displayed oldest-first after reversal).
     */
    void SetHistory(const std::vector<Message>& messages);

    /**
     * @brief Sets the use case for loading message history.
     * @param loader Pointer to GetMessagesUseCase for auto-refresh.
     */
    void SetMessageLoader(GetMessagesUseCase* loader);

private:
    const ChatWindowStyle& m_current_style; ///< Visual style reference (owned by StyleManager).

    /** 
     * @brief Formats a time_point as "HH:MM" string. 
     */
    std::string formatTime(const std::chrono::system_clock::time_point& tp) const;

    // State
    ISendMessageHandler* m_handler = nullptr;
    GetMessagesUseCase* m_message_loader = nullptr;
    std::string m_sender_id;        ///< Current user ID.
    std::string m_receiver_id;      ///< Chat partner ID.
    std::string m_receiver_name;    ///< Chat partner display name.
    char m_input_buffer[1024] = ""; ///< Input text buffer.

    /** 
     * @brief Internal representation of a displayed message. 
     */
    struct DisplayMessage {
        std::string id;
        std::string text;
        bool is_local;
        std::chrono::system_clock::time_point timestamp;
        MessageStatus status = MessageStatus::Default;
    };
    std::vector<DisplayMessage> m_messages; ///< Current visible messages.
    std::mutex m_mutex;                     ///< Mutex for thread-safe access to messages and user info.
    double m_last_message_refresh = 0.0;    ///< Timestamp of last auto-refresh (seconds from ImGui::GetTime()).
    static constexpr double REFRESH_INTERVAL = 5.0; ///< Auto-refresh interval in seconds.

    // Rendering helpers
    void renderHeader();
    void renderMessages(const std::vector<DisplayMessage>& messages);
    void renderMessageBubble(const DisplayMessage& dm, float max_width, float available_width);
    void renderInputArea(bool has_handler, const std::string& sender_id, const std::string& receiver_id);
};