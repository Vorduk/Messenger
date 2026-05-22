#include "LoginWindow.h"
#include "imgui.h"
#include <cstring>

LoginWindow::LoginWindow(LoginUseCase& login_uc, RegisterUseCase& register_uc, LoginCallback on_login)
    : m_login_uc(login_uc), m_register_uc(register_uc), m_on_login(std::move(on_login)) {
    memset(m_username, 0, sizeof(m_username));
}

void LoginWindow::render() {
    ImGui::Text("Welcome to Messenger!");
    if (m_is_waiting) {
        ImGui::Text("Connecting to server...");
        return;
    }
    ImGui::InputText("Username", m_username, IM_ARRAYSIZE(m_username));
    if (ImGui::Button("Login")) {
        if (strlen(m_username) == 0) {
            m_error_message = "Username cannot be empty";
        }
        else {
            m_is_waiting = true;
            m_error_message.clear();
            m_login_uc.execute(m_username,
                [this](bool success, const std::string& userIdOrError) {
                    m_is_waiting = false;
                    if (success) m_on_login(userIdOrError, m_username); // Pass username for reconnection
                    else m_error_message = userIdOrError;
                });
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Register")) {
        if (strlen(m_username) == 0) {
            m_error_message = "Username cannot be empty";
        }
        else {
            m_is_waiting = true;
            m_error_message.clear();
            m_register_uc.execute(m_username,
                [this](bool success, const std::string& userIdOrError) {
                    m_is_waiting = false;
                    if (success) m_on_login(userIdOrError, m_username); // Pass username for reconnection
                    else m_error_message = userIdOrError;
                });
        }
    }
    if (!m_error_message.empty()) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", m_error_message.c_str());
    }
}