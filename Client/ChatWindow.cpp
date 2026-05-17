#include "ChatWindow.h"
#include "imgui.h"
#include <cstring>

ChatWindow::ChatWindow() {
    std::memset(m_input_buffer, 0, sizeof(m_input_buffer));
}

void ChatWindow::SetHandler(ISendMessageHandler* handler) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_handler = handler;
}

void ChatWindow::SetUsers(const std::string& sender_id,
    const std::string& receiver_id,
    const std::string& receiver_name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sender_id = sender_id;
    m_receiver_id = receiver_id;
    m_receiver_name = receiver_name;
    m_messages.clear();
}

void ChatWindow::AddMessage(const std::string& sender,
    const std::string& text,
    bool is_local) {
    bool local = is_local ? is_local : (sender == m_sender_id);
    std::lock_guard<std::mutex> lock(m_mutex);
    m_messages.push_back({ text, local });
}

void ChatWindow::SetHistory(const std::vector<Message>& messages) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_messages.clear();
    for (size_t i = 0; i < messages.size(); ++i) {
        const Message& msg = messages[i];
        bool local = (msg.sender_id == m_sender_id);
        m_messages.push_back({ msg.text, local });
    }
}

void ChatWindow::AddConfirmation(const std::string& text) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_status_messages.push_back("[OK] " + text);
    if (m_status_messages.size() > 20) {
        m_status_messages.erase(m_status_messages.begin());
    }
}

void ChatWindow::AddError(const std::string& text) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_status_messages.push_back("[ERR] " + text);
    if (m_status_messages.size() > 20) {
        m_status_messages.erase(m_status_messages.begin());
    }
}

void ChatWindow::SetMessageLoader(GetMessagesUseCase* loader) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_message_loader = loader;
}

void ChatWindow::Render() {

    std::vector<DisplayMessage> messages;
    std::vector<std::string> status_messages;
    std::string receiver_name;
    std::string sender_id;
    std::string receiver_id;
    bool has_handler = false;
    GetMessagesUseCase* loader = nullptr;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        messages = m_messages;
        status_messages = m_status_messages;
        receiver_name = m_receiver_name;
        sender_id = m_sender_id;
        receiver_id = m_receiver_id;
        has_handler = (m_handler != nullptr);
        loader = m_message_loader;
    }

    if (loader && !receiver_id.empty()) {
        double now = ImGui::GetTime();
        if (now - m_last_message_refresh > REFRESH_INTERVAL) {
            m_last_message_refresh = now;
            loader->execute(sender_id, receiver_id, 50, 0,
                [this](std::vector<Message> newMessages) {
                    SetHistory(newMessages);
                });
        }
    }

    if (!receiver_name.empty()) {
        ImGui::Text("Chat with %s", receiver_name.c_str());
    }
    else {
        ImGui::Text("Select a contact to start chatting");
    }
    ImGui::Separator();

    const float footer_height = ImGui::GetFrameHeightWithSpacing() * 2 + 30.0f;
    ImGui::BeginChild("ChatHistory", ImVec2(0, -footer_height), true);

    for (size_t i = 0; i < messages.size(); ++i) {
        const DisplayMessage& dm = messages[i];
        if (dm.is_local) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
            float text_width = ImGui::CalcTextSize(dm.text.c_str()).x;
            ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - text_width - 10.0f);
            ImGui::TextWrapped("%s", dm.text.c_str());
            ImGui::PopStyleColor();
        }
        else {
            ImGui::TextWrapped("%s", dm.text.c_str());
        }
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5.0f) {
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();


    ImGui::PushItemWidth(-1);
    bool send_pressed = ImGui::InputText("##MessageInput",
        m_input_buffer,
        IM_ARRAYSIZE(m_input_buffer),
        ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button("Send") || send_pressed) {
        if (has_handler && strlen(m_input_buffer) > 0) {
            if (!sender_id.empty() && !receiver_id.empty()) {
                m_handler->onUserSendMessage(sender_id, receiver_id, m_input_buffer);
                std::memset(m_input_buffer, 0, sizeof(m_input_buffer));
            }
            else {
                
                std::lock_guard<std::mutex> lock(m_mutex);
                m_status_messages.push_back("[ERR] Select a contact first");
                if (m_status_messages.size() > 20)
                    m_status_messages.erase(m_status_messages.begin());
            }
        }
    }


    if (!status_messages.empty()) {
        ImGui::Separator();
        ImGui::Text("Status:");
        for (size_t i = 0; i < status_messages.size(); ++i) {
            const std::string& msg = status_messages[i];
            ImGui::BulletText("%s", msg.c_str());
        }
    }
}