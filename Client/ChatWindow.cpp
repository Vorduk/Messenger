#include "ChatWindow.h"
#include "imgui.h"
#include <cstring>

ChatWindow::ChatWindow(ISendMessageHandler& send_handler)
    : m_send_handler(send_handler) {
    memset(m_input_buffer, 0, sizeof(m_input_buffer));
}

void ChatWindow::Render() {
    ImGui::InputText("Message", m_input_buffer, IM_ARRAYSIZE(m_input_buffer));
    if (ImGui::Button("Send")) {
        if (strlen(m_input_buffer) > 0) {
            m_send_handler.onUserSendMessage("User", "Server", m_input_buffer);
            memset(m_input_buffer, 0, sizeof(m_input_buffer));
        }
    }
    ImGui::Separator();
    ImGui::Text("Status:");
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& msg : m_status_messages) {
            ImGui::BulletText("%s", msg.c_str());
        }
    }
}

void ChatWindow::AddConfirmation(const std::string& text) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_status_messages.push_back("[OK] " + text);
    if (m_status_messages.size() > 20)
        m_status_messages.erase(m_status_messages.begin());
}

void ChatWindow::AddError(const std::string& text) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_status_messages.push_back("[ERR] " + text);
    if (m_status_messages.size() > 20)
        m_status_messages.erase(m_status_messages.begin());
}