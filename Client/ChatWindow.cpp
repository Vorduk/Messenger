#include "ChatWindow.h"
#include "imgui.h"
#include <cstring>
#include <chrono>
#include <ctime>

ChatWindow::ChatWindow(const ChatWindowStyle& current_style) 
    : m_current_style(current_style)
{
    std::memset(m_input_buffer, 0, sizeof(m_input_buffer));
}

const char* ChatWindow::getName() const
{
    return "Chat";
}

void ChatWindow::render() {

    std::vector<DisplayMessage> messages;
    std::string receiver_name;
    std::string sender_id;
    std::string receiver_id;
    bool has_handler = false;
    GetMessagesUseCase* loader = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        messages = m_messages;
        receiver_name = m_receiver_name;
        sender_id = m_sender_id;
        receiver_id = m_receiver_id;
        has_handler = (m_handler != nullptr);
        loader = m_message_loader;
    }

    // Not blocking UI
    if (loader && !receiver_id.empty()) {
        double now = ImGui::GetTime();  // Current time in seconds
        if (now - m_last_message_refresh > REFRESH_INTERVAL) {
            m_last_message_refresh = now;  // Remember this update time
            loader->execute(sender_id, receiver_id, 50, 0,
                [this](std::vector<Message> newMessages) {
                    SetHistory(newMessages);  // Refreshes when server gives respose
                });
        }
    }

    renderHeader();
    ImGui::PushStyleColor(ImGuiCol_Separator, m_current_style.separator_color);
    ImGui::Separator();
    ImGui::PopStyleColor();
    renderMessages(messages);
    renderInputArea(has_handler, sender_id, receiver_id);
}

void ChatWindow::renderMessages(const std::vector<DisplayMessage>& messages) {
    const float footer_height = ImGui::GetFrameHeightWithSpacing() * 2 + 30.0f;
    ImGui::BeginChild("ChatHistory", ImVec2(0, -footer_height), true);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, m_current_style.chat_background_color);

    float available_width = ImGui::GetContentRegionAvail().x;
    float max_width = available_width * m_current_style.message_style.max_text_width_ratio;

    for (const auto& dm : messages) {
        renderMessageBubble(dm, max_width, available_width);
        ImGui::Spacing();
    }

    // Автопрокрутка
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5.0f) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::PopStyleColor();
    ImGui::EndChild();
}

void ChatWindow::renderMessageBubble(const DisplayMessage& dm, float max_width, float available_width) {
    // Позиционирование
    if (dm.is_local) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available_width - max_width);
    }

    float padding = m_current_style.message_style.bubble_padding;
    float text_wrap_width = max_width - padding * 2;
    ImVec2 text_size = ImGui::CalcTextSize(dm.text.c_str(), nullptr, false, text_wrap_width);

    // Ширина нижней строки: время + статус
    std::string status_str = messageStatusToString(dm.status);
    float status_width = status_str.empty() ? 0 : ImGui::CalcTextSize(status_str.c_str()).x + 4.0f;
    std::string time_str = formatTime(dm.timestamp);
    float time_width = ImGui::CalcTextSize(time_str.c_str()).x;
    float bottom_row_width = time_width + status_width;

    float block_height = text_size.y + ImGui::GetTextLineHeightWithSpacing() + padding * 2;

    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + text_wrap_width);

    ImVec4 bg_color = dm.is_local ? m_current_style.message_style.sender_bg_color : m_current_style.message_style.reciever_bg_color;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, bg_color);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, m_current_style.message_style.border_width);
    ImGui::PushStyleColor(ImGuiCol_Border, m_current_style.message_style.border_color);

    std::string child_id = "msg_" + dm.id;
    ImGui::BeginChild(child_id.c_str(), ImVec2(max_width, block_height), false, ImGuiWindowFlags_NoScrollbar);

    // Текст
    ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.message_style.text_style.color);
    ImGui::TextWrapped("%s", dm.text.c_str());
    ImGui::PopStyleColor();

    // Нижняя строка: время + статус, выровненные вправо
    float bottom_y = block_height - ImGui::GetTextLineHeightWithSpacing() - padding;
    ImGui::SetCursorPos(ImVec2(padding, bottom_y));
    float avail_bottom = max_width - padding * 2;
    float cursor_x = avail_bottom - bottom_row_width;
    ImGui::SetCursorPosX(padding + cursor_x);

    ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.message_style.time_text_style.color);
    ImGui::Text("%s", time_str.c_str());
    ImGui::PopStyleColor();

    if (!status_str.empty()) {
        ImGui::SameLine();
        int status_idx = static_cast<int>(dm.status);
        if (status_idx < 0 || status_idx > 5) status_idx = 5;
        ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.message_style.status_style[status_idx].color);
        ImGui::Text("%s", status_str.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::EndChild();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
    ImGui::PopTextWrapPos();
}

void ChatWindow::renderInputArea(bool has_handler, const std::string& sender_id, const std::string& receiver_id) {
    ImGui::PushStyleColor(ImGuiCol_FrameBg, m_current_style.input_background_color);
    ImGui::PushItemWidth(-1);
    bool send_pressed = ImGui::InputTextWithHint("##MessageInput",
        m_current_style.input_hint_text.c_str(),
        m_input_buffer, IM_ARRAYSIZE(m_input_buffer),
        ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::PopItemWidth();
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, m_current_style.send_button_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, m_current_style.send_button_hover_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, m_current_style.send_button_active_color);
    ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.send_button_text_style.color);

    if (ImGui::Button(m_current_style.send_button_text.c_str()) || send_pressed) {
        if (has_handler && strlen(m_input_buffer) > 0) {
            if (!sender_id.empty() && !receiver_id.empty()) {
                std::string text = m_input_buffer;
                std::string temp_id = "temp_" +
                    std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
                AddMessage(sender_id, text, true, std::chrono::system_clock::now(), temp_id, MessageStatus::Sending);
                m_handler->onUserSendMessage(sender_id, receiver_id, text);
                std::memset(m_input_buffer, 0, sizeof(m_input_buffer));
            }
        }
    }
    ImGui::PopStyleColor(4);
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
    bool is_local,
    const std::chrono::system_clock::time_point& timestamp,
    const std::string& msg_id,
    MessageStatus status) {
    bool local = is_local ? is_local : (sender == m_sender_id);
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string id = msg_id;
    if (id.empty()) {
        id = "msg_" + std::to_string(m_messages.size()) + "_" +
            std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    }

    m_messages.push_back({ id, text, local, timestamp, status });
}

void ChatWindow::SetHistory(const std::vector<Message>& messages) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_messages.clear();
    for (size_t i = messages.size(); i > 0; --i) {
        const Message& msg = messages[i - 1];
        bool local = (msg.sender_id == m_sender_id);
        m_messages.push_back({ msg.id, msg.text, local, msg.timestamp, msg.status });
    }
}

void ChatWindow::SetMessageLoader(GetMessagesUseCase* loader) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_message_loader = loader;
}

void ChatWindow::renderHeader() {
    if (!m_receiver_name.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.chat_header_text_style.color);
        ImGui::Text("%s%s", m_current_style.chat_header_prefix.c_str(), m_receiver_name.c_str());
        ImGui::PopStyleColor();
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.empty_chat_text_style.color);
        ImGui::Text("%s", m_current_style.empty_chat_text.c_str());
        ImGui::PopStyleColor();
    }
}

std::string ChatWindow::formatTime(const std::chrono::system_clock::time_point& tp) const {
    using namespace std::chrono;
    auto t = system_clock::to_time_t(tp);
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[16];
    std::strftime(buf, sizeof(buf), "%H:%M", &tm);
    return std::string(buf);
}
