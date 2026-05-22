#pragma once
#include "DockableWindow.h"
#include "GetUsersUseCase.h"
#include <string>
#include <vector>
#include <functional>
#include "StyleManager.h"
#include "ChatListItem.h"

/**
 * @brief ImGui window with list of chats.
 */
class ChatListWindow : public DockableWindow {
public:
    ChatListWindow(GetUsersUseCase& get_users_uc, const std::string& current_user_id);
    const char* getName() const override { return "Chat List"; }
    void render() override;
    void refreshUsers();
    void setOnUserSelected(std::function<void(const User&)> callback);

private:
    // Render.
    void renderUserBlock(const ChatListItem& item);
    void renderUserAvatar(const User& user, float size);
    ChatListWindowStyle m_current_style;

    GetUsersUseCase& m_get_users_uc;
    std::string m_current_user_id;
    std::vector<ChatListItem> m_items;
    std::function<void(const User&)> m_on_selected;

    std::string formatLastMessageTime(const std::chrono::system_clock::time_point& tp) const;
};