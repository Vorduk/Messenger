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
 * @brief ImGui window displaying the chat list and user search.
 *
 * Shows two tabs: "Chats" (existing conversations with last message preview,
 * status, and timestamp) and "Users" (server-side search for starting new chats).
 * Supports manual refresh via refreshUsers().
 * All server calls are asynchronous and non-blocking.
 */
class ChatListWindow : public DockableWindow {
public:
    /**
     * @brief Constructs the chat list window.
     * @param current_style Style settings for rendering.
     * @param get_users_uc Use case for searching users.
     * @param get_chats_uc Use case for loading chat list.
     * @param local_repo Local storage for caching chats.
     * @param current_user_id ID of the logged-in user.
     */
    ChatListWindow(const ChatListWindowStyle& current_style, GetUsersUseCase& get_users_uc, GetChatsUseCase& get_chats_uc, IMessageRepository& local_repo, const std::string& current_user_id);

    /** 
     * @return Window title used by the docking system. 
     */
    const char* getName() const override;

    /** 
     * @brief Renders the chat list UI. Called every frame. 
     */
    void render() override;

    /**
     * @brief Sets the callback invoked when a user is selected.
     * @param callback Receives the selected User object.
     */
    void setOnUserSelected(std::function<void(const User&)> callback);

    /**
     * @brief Manually refreshes the chat list from server (or cache on failure).
     * Also resets the auto-refresh timer.
     */
    void refreshUsers();

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

    /**
     * @brief Loads the chat list from server, falling back to local cache on failure.
     *
     * Asynchronous. Updates m_chats, m_is_offline, and caches results on success.
     */
    void loadChats();

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

    // Tabs.
    enum class Tab { Chats, Users };
    Tab m_active_tab = Tab::Chats;

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

    std::vector<ChatListItem> m_chats;              ///< Current chat list (server or cached).
    std::vector<ChatListItem> m_search_results;     ///< Results from the last searchUsers() call.
    std::function<void(const User&)> m_on_selected; ///< Callback for user selection.
    char m_search_buffer[256] = "";                 ///< Input buffer for the search field.
    const ChatListWindowStyle& m_current_style;     ///< Visual style reference (owned by StyleManager).

    // Use cases.
    GetUsersUseCase& m_get_users_uc;
    GetChatsUseCase& m_get_chats_uc;

    IMessageRepository& m_local_repo;   ///< Local cache.
    std::string m_current_user_id;      ///< Id of current user.

    bool m_is_offline = false;          ///< True when showing cached data (server unreachable).
    double m_last_chats_refresh = 0.0;  ///< Timestamp of last auto-refresh (seconds from ImGui::GetTime()).

    bool m_is_user_search_failed = false;    ///< User search failed

};