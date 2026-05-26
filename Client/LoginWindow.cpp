#include "LoginWindow.h"
#include "imgui.h"
#include <cstring>

LoginWindow::LoginWindow(LoginUseCase& login_uc, RegisterUseCase& register_uc, LoginCallback on_login)
    : m_login_uc(login_uc), m_register_uc(register_uc), m_on_login(std::move(on_login)) {
    memset(m_username, 0, sizeof(m_username));
    memset(m_display_name, 0, sizeof(m_display_name));
    memset(m_birthday, 0, sizeof(m_birthday));
}

const char* LoginWindow::getName() const
{
    return "Login";
}

void LoginWindow::render() {
    ImGui::Text("Welcome to Messenger!");
    if (m_is_waiting) {
        ImGui::Text("Connecting to server...");
        return;
    }

    ImGui::InputText("Username", m_username, IM_ARRAYSIZE(m_username));
    ImGui::InputText("Display Name", m_display_name, IM_ARRAYSIZE(m_display_name));   // new field
    ImGui::InputText("Birthday (YYYY-MM-DD)", m_birthday, IM_ARRAYSIZE(m_birthday));   // new field

    if (ImGui::Button("Login")) {
        if (strlen(m_username) == 0) {
            m_error_message = "Username cannot be empty";
        }
        else {
            m_is_waiting = true;
            m_error_message.clear();
            m_login_uc.execute(m_username,
                [this](bool success, const std::string& user_id_or_error, const std::string& display_name) {
                    m_is_waiting = false;
                    if (success) m_on_login(user_id_or_error, m_username, display_name);
                    else m_error_message = user_id_or_error;
                });
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Register")) {
        if (strlen(m_username) == 0) {
            m_error_message = "Username cannot be empty";
        }
        else if (strlen(m_display_name) == 0) {                     // display name required for registration
            m_error_message = "Display name cannot be empty";
        }
        else if (strlen(m_birthday) == 0) {                         // birthday required for registration
            m_error_message = "Birthday cannot be empty";
        }
        else {
            m_is_waiting = true;
            m_error_message.clear();
            m_register_uc.execute(m_username, m_display_name, m_birthday,
                [this](bool success, const std::string& user_id_or_error, const std::string& display_name) {
                    m_is_waiting = false;
                    if (success) m_on_login(user_id_or_error, m_username, display_name);
                    else m_error_message = user_id_or_error;
                });
        }
    }

    if (!m_error_message.empty()) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", m_error_message.c_str());
    }
}