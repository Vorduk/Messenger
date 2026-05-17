#pragma once
#include "IDockableWindow.h"
#include "GetUsersUseCase.h"
#include <string>
#include <vector>
#include <functional>

class ChatListWindow : public IDockableWindow {
public:
    ChatListWindow(GetUsersUseCase& get_users_uc, const std::string& current_user_id);
    const char* getName() const override { return "Chat List"; }
    void Render() override;
    void SetOnUserSelected(std::function<void(const User&)> callback);

private:
    void refreshUsers();
    GetUsersUseCase& m_get_users_uc;
    std::string m_current_user_id;
    std::vector<User> m_users;
    std::function<void(const User&)> m_on_selected;
};