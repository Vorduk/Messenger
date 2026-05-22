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
    ChatListWindow(GetUsersUseCase& get_users_uc, GetChatsUseCase& get_chats_uc, IMessageRepository& local_repo, const std::string& current_user_id);
    const char* getName() const override { return "Chat List"; }
    void render() override;

    void setOnUserSelected(std::function<void(const User&)> callback);
    void refreshUsers();
    void loadChats();
    void updateLastMessage(const std::string& partner_id, const std::string& text, const std::chrono::system_clock::time_point& timestamp);

private:
    enum class Tab { Chats, Users };
    Tab m_activeTab = Tab::Chats;

    void searchUsers(const std::string& query);

    // Render.
    void renderSearchBar();
    void renderTabs();
    void renderChatList();
    void renderUserList();
    void renderUserBlock(const ChatListItem& item);
    void renderUserAvatar(const User& user, float size);

    std::string formatLastMessageTime(const std::chrono::system_clock::time_point& tp) const;

    GetUsersUseCase& m_get_users_uc;
    GetChatsUseCase& m_get_chats_uc;
    IMessageRepository& m_local_repo;
    std::string m_current_user_id;

    std::vector<ChatListItem> m_chats;
    std::vector<ChatListItem> m_search_results;
    std::function<void(const User&)> m_on_selected;

    char m_search_buffer[256] = "";
    ChatListWindowStyle m_current_style;

    bool m_is_offline = false;
    double m_last_chats_refresh = 0.0;
};