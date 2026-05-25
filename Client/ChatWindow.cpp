#include "ChatWindow.h"
#include "imgui.h"
#include <cstring>
#include <chrono>
#include <ctime>

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
    bool is_local,
    const std::chrono::system_clock::time_point& timestamp,
    const std::string& msg_id) {
    bool local = is_local ? is_local : (sender == m_sender_id);
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string id = msg_id;
    if (id.empty()) {
        // Fallback, но лучше всегда передавать ID
        id = "msg_" + std::to_string(m_messages.size()) + "_" +
            std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    }

    m_messages.push_back({ id, text, local, timestamp });
}

void ChatWindow::SetHistory(const std::vector<Message>& messages) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_messages.clear();
    // Идём с конца (старые -> новые)
    for (size_t i = messages.size(); i > 0; --i) {
        const Message& msg = messages[i - 1];
        bool local = (msg.sender_id == m_sender_id);
        // Используем настоящий ID сообщения из БД
        m_messages.push_back({ msg.id, msg.text, local, msg.timestamp });
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

void ChatWindow::SetMessageLoader(GetMessagesUseCase* loader) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_message_loader = loader;
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

const char* ChatWindow::getName() const
{
    return "Chat";
}

void ChatWindow::render() {
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

    // Автообновление истории (каждые 5 секунд)
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
        float available = ImGui::GetContentRegionAvail().x;
        float wrap_width = available * 0.7f;   // ширина текстового блока

        // Сдвиг своих сообщений вправо
        if (dm.is_local) {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available * 0.3f);
        }

        // Рассчитываем высоту текста с учётом переноса по ширине wrap_width
        // Вычитаем FramePadding, чтобы текст не прилипал к границам
        float text_wrap_width = wrap_width - ImGui::GetStyle().FramePadding.x * 2;
        ImVec2 text_size = ImGui::CalcTextSize(dm.text.c_str(), nullptr, false, text_wrap_width);

        // Высота блока: высота текста + строка для времени + отступы
        float block_height = text_size.y + ImGui::GetTextLineHeightWithSpacing() +
            ImGui::GetStyle().FramePadding.y * 2;

        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + text_wrap_width);

        ImVec4 bg_color = dm.is_local ? ImVec4(0.2f, 0.6f, 0.2f, 0.3f)
            : ImVec4(0.3f, 0.3f, 0.3f, 0.3f);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, bg_color);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));

        std::string child_id = "msg_" + dm.id;

        // Явно указываем фиксированную высоту, чтобы избежать растягивания на весь экран
        ImGui::BeginChild(child_id.c_str(), ImVec2(wrap_width, block_height),
            false, ImGuiWindowFlags_NoScrollbar);

        ImGui::TextWrapped("%s", dm.text.c_str());

        std::string time_str = formatTime(dm.timestamp);
        float time_width = ImGui::CalcTextSize(time_str.c_str()).x;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - time_width);
        ImGui::Text("%s", time_str.c_str());

        ImGui::EndChild();
        ImGui::PopStyleColor(2);   // ChildBg, Border
        ImGui::PopStyleVar();      // ChildBorderSize
        ImGui::PopTextWrapPos();

        ImGui::Spacing();
    }

    // Автопрокрутка
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5.0f) {
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();

    // Поле ввода и кнопка Send
    ImGui::PushItemWidth(-1);
    bool send_pressed = ImGui::InputText("##MessageInput",
        m_input_buffer, IM_ARRAYSIZE(m_input_buffer),
        ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button("Send") || send_pressed) {
        if (has_handler && strlen(m_input_buffer) > 0) {
            if (!sender_id.empty() && !receiver_id.empty()) {
                std::string text = m_input_buffer;
                // Генерируем временный ID для мгновенного отображения
                std::string temp_id = "temp_" +
                    std::to_string(
                        std::chrono::system_clock::now().time_since_epoch().count());
                // Добавляем локальное сообщение сразу в историю
                AddMessage(sender_id, text, true,
                    std::chrono::system_clock::now(), temp_id);

                // Отправляем на сервер
                m_handler->onUserSendMessage(sender_id, receiver_id, text);
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
        for (const auto& msg : status_messages) {
            ImGui::BulletText("%s", msg.c_str());
        }
    }
}