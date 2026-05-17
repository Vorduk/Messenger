#include "ChatListWindow.h"
#include "imgui.h"
#include "LogMacros.h"

ChatListWindow::ChatListWindow(GetUsersUseCase& get_users_uc, const std::string& current_user_id)
    : m_get_users_uc(get_users_uc), m_current_user_id(current_user_id) {
    refreshUsers();
}

void ChatListWindow::Render() {
    if (ImGui::Button("Refresh")) {
        refreshUsers();
    }

    ImGui::Separator();

    if (m_users.empty()) {
        ImGui::Text("No users found.");
    }
    else {
        for (const User& user : m_users) {
            std::string label = user.display_name.empty() ? user.username : user.display_name;
            if (user.is_online)
                label += " (online)";
            else
                label += " (offline)";

            if (ImGui::Selectable(label.c_str(), false)) {
                if (m_on_selected) {
                    m_on_selected(user);
                }
            }
        }
    }
}

void ChatListWindow::SetOnUserSelected(std::function<void(const User&)> callback) {
    m_on_selected = std::move(callback);
}

void ChatListWindow::refreshUsers() {
    m_get_users_uc.execute(m_current_user_id,
        [this](std::vector<User> users) {
            m_users = std::move(users);
            LOG_INFO("User list updated, count: {}", m_users.size());
        });
}