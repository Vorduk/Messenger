#pragma once
#include "DockableWindow.h"
#include "GetUsersUseCase.h"
#include <string>
#include <vector>
#include <functional>
#include "StyleManager.h"

/**
 * @brief ImGui window with list of chats.
 */
class ChatListWindow : public DockableWindow {
public:
    ChatListWindow(GetUsersUseCase& get_users_uc, const std::string& current_user_id);
    const char* getName() const override { return "Chat List"; }
    void render() override;

    void setOnUserSelected(std::function<void(const User&)> callback);

private:
    void refreshUsers();

    // Render.
    void renderUserBlock(const User& user);
    void renderUserAvatar(const User& user, float size);
    ChatListWindowStyle m_current_style;

    GetUsersUseCase& m_get_users_uc;
    std::string m_current_user_id;
    std::vector<User> m_users;
    std::function<void(const User&)> m_on_selected;


};