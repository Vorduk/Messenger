#pragma once
#include "DockableWindow.h"
#include "GetUsersUseCase.h"
#include <string>
#include <vector>
#include <functional>
#include "StyleManager.h"
#include "ChatListItem.h"
#include "GetChatsUseCase.h"
#include <IMessageRepository.h>


/**
 * @brief ImGui window with list of chats.
 */
class ChatListWindow : public DockableWindow {
public:
    ChatListWindow(const ChatListWindowStyle& current_style, GetUsersUseCase& get_users_uc, GetChatsUseCase& get_chats_uc, IMessageRepository& local_repo, const std::string& current_user_id);
    const char* getName() const override;
    void render() override;

    void setOnUserSelected(std::function<void(const User&)> callback);
    void refreshUsers();

    void loadChats();

    /**
     * @brief Updates the last message for the chat with the given partner.
     *
     * If the chat is found, it is moved to the top of the list.
     * @param partner_id the other user's ID.
     * @param text message text to display as preview.
     * @param timestamp when the message was sent/received.
     * @param message_status delivery status of the message.
     */
    void updateLastMessage(const std::string& partner_id, const std::string& text, const std::chrono::system_clock::time_point& timestamp, MessageStatus message_status);

private:
    enum class Tab { Chats, Users };
    Tab m_activeTab = Tab::Chats;

    /**
     * @brief Searches for users on the server by username or display name.
     *
     * Sends an asynchronous search request to the server and immediately returns.
     * The provided callback is stored internally and executed later when the server
     * responds, without blocking the UI thread.
     * 
     * @param query Search string.
     *
     * @note Non-blocking. The function returns immediately; results appear
     * in the next frame(s) after the server responds.
     */
    void searchUsers(const std::string& query);

    // Rendering helpers.
    void renderSearchBar();
    void renderTabs();
    void renderChatList();
    void renderUserList();
    void renderUserBlock(const ChatListItem& item);
    void renderUserAvatar(const User& user, float size);

    /**
     * @brief Formats a message timestamp into a readable relative time string.
     *
     * Formatting rules (applied in order):
     * - If the timestamp is time_point::min() (default/unset), an empty string
     *   is returned.
     * - If the message was sent today (same calendar date): returns time in
     *   "HH:MM" format, e.g. "14:32".
     * - If the message was sent yesterday through 6 days ago (1–6 days
     *   difference): returns "Last week".
     * - If the message was sent this year but more than 6 days ago: returns
     *   the abbreviated month name, e.g. "Jan", "Sept".
     * - If the message was sent last year (exactly 1 year difference):
     *   returns "Last year".
     * - If the message was sent 2 or more years ago: returns the number of
     *   years followed by "years ago", e.g. "2 years ago".
     *
     * @param tp The timestamp to format. Use @c time_point::min() for unset values.
     * @return A human-readable relative time string, or an empty string if
     *         the timestamp is unset.
     *
     * @note Uses local time (respects system timezone/DST settings via
     *       @c localtime_s on Windows or @c localtime_r on other platforms).
     * @note Day difference is calculated using calendar midnights, not exact
     *       24‑hour intervals. This means "today at 23:59" and "tomorrow at 00:01"
     *       are correctly reported as different days.
     */
    std::string formatLastMessageTime(const std::chrono::system_clock::time_point& tp) const;

    GetUsersUseCase& m_get_users_uc;
    GetChatsUseCase& m_get_chats_uc;
    IMessageRepository& m_local_repo;
    std::string m_current_user_id;

    std::vector<ChatListItem> m_chats;
    std::vector<ChatListItem> m_search_results;
    std::function<void(const User&)> m_on_selected;

    char m_search_buffer[256] = "";
    const ChatListWindowStyle& m_current_style;

    bool m_is_offline = false;
    double m_last_chats_refresh = 0.0;
};