#include "LoginWindow.h"
#include "imgui.h"
#include <cstring>

LoginWindow::LoginWindow(UserManager& user_manager, LoginCallback on_login)
    : m_user_manager(user_manager), m_on_login(std::move(on_login)) 
{
    std::memset(m_username_buffer, 0, sizeof(m_username_buffer));
}

void LoginWindow::Render() {
    ImGui::Text("Welcome to Messenger!");
    ImGui::InputText("Username", m_username_buffer, IM_ARRAYSIZE(m_username_buffer));

    if (ImGui::Button("Login")) {
        if (m_user_manager.login(m_username_buffer)) {
            m_on_login();
        }
        else {
            m_is_login_failed = true;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Register")) {
        if (m_user_manager.registerUser(m_username_buffer)) {
            m_on_login();
        }
        else {
            m_is_login_failed = true;
        }
    }

    if (m_is_login_failed) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Login/Registration failed. User may already exist.");
    }
}