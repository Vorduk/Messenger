#include "ChatListWindow.h"
#include "imgui.h"
#include "LogMacros.h"

ChatListWindow::ChatListWindow(GetUsersUseCase& get_users_uc, const std::string& current_user_id)
    : m_get_users_uc(get_users_uc), m_current_user_id(current_user_id)
{
    m_current_style = StyleManager::getInstance().getChatListWindowStyle();
    refreshUsers();
}

void ChatListWindow::setOnUserSelected(std::function<void(const User&)> callback) {
    m_on_selected = std::move(callback);
}

void ChatListWindow::refreshUsers() {
    m_get_users_uc.execute(m_current_user_id,
        [this](std::vector<ChatListItem> items) {
            m_items = std::move(items);
            LOG_INFO("User list updated, count: {}", m_items.size());
        });
}

void ChatListWindow::render() {
    // Use refresh button text and colors from style
    ImGui::PushStyleColor(ImGuiCol_Button, m_current_style.refresh_button_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, m_current_style.refresh_button_hover_color);

    if (ImGui::Button(m_current_style.refresh_button_text.c_str())) {
        refreshUsers();
    }

    ImGui::PopStyleColor(2);

    ImGui::Separator();

    if (m_items.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.empty_chat_list_text_style.color);
        ImGui::Text("%s", m_current_style.empty_chat_list_text.c_str());
        ImGui::PopStyleColor();
    }
    else {
        for (const auto& item : m_items) {
            renderUserBlock(item);
            ImGui::Spacing();
        }
    }
}

void ChatListWindow::renderUserBlock(const ChatListItem& item)
{
    const User& user = item.user;
    std::string block_id = "user_block_" + user.id; // Unique name for imgui.

    ImVec2 block_start = ImGui::GetCursorScreenPos();
    float block_width = ImGui::GetContentRegionAvail().x;

    // Use block size from style (height from style, width auto).
    float block_height = m_current_style.user_block_style.size.y;
    ImGui::BeginChild(block_id.c_str(), ImVec2(block_width, block_height), false, ImGuiWindowFlags_NoScrollbar);

    // Invicible button for user selection
    ImGui::SetCursorPos(ImVec2(0, 0));
    if (ImGui::InvisibleButton(("select_" + user.id).c_str(), ImVec2(block_width, block_height))) {
        if (m_on_selected) {
            m_on_selected(user);
        }
    }

    // Hover style.
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 win_pos = ImGui::GetWindowPos();
    ImVec2 win_size = ImGui::GetWindowSize();
    if (ImGui::IsWindowHovered()) {
        draw_list->AddRectFilled(win_pos, ImVec2(win_pos.x + win_size.x, win_pos.y + win_size.y), ImGui::ColorConvertFloat4ToU32(m_current_style.user_block_style.background_hover));
    }

    // Use padding from style for avatar position.
    ImGui::SetCursorPos(ImVec2(m_current_style.user_block_style.padding, m_current_style.user_block_style.padding));
    ImVec2 avatar_cursor = ImGui::GetCursorScreenPos(); // remember screen position for online indicator.
    renderUserAvatar(user, m_current_style.avatar_style.size);

    // Draw online indicator (green circle) at bottom-right of avatar.
    if (user.is_online) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        float size = m_current_style.avatar_style.size;
        ImVec2 center(avatar_cursor.x + size - m_current_style.online_indicator_radius,
            avatar_cursor.y + size - m_current_style.online_indicator_radius);
        draw_list->AddCircleFilled(center, m_current_style.online_indicator_radius,
            ImGui::ColorConvertFloat4ToU32(m_current_style.online_indicator_color));
        // white border for better contrast
        draw_list->AddCircle(center, m_current_style.online_indicator_radius, IM_COL32_WHITE, 0, 1.5f);
    }

    // Calculate name position based on avatar size and padding.
    float name_x = m_current_style.avatar_style.size + m_current_style.user_block_style.padding * 2;
    float name_y = m_current_style.user_block_style.padding;
    ImGui::SetCursorPos(ImVec2(name_x, name_y));

    // Render name with style.
    std::string display_name = user.display_name.empty() ? user.username : user.display_name;
    ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.user_name_text_style.color);
    ImGui::Text("%s", display_name.c_str());
    ImGui::PopStyleColor();

    // Render last message time on the right side (opposite to name).
    std::string time_str = formatLastMessageTime(item.last_message_time);
    if (!time_str.empty()) {
        ImGui::SameLine();
        float time_width = ImGui::CalcTextSize(time_str.c_str()).x;
        float right_padding = m_current_style.user_block_style.padding;
        ImGui::SetCursorPosX(block_width - right_padding - time_width);
        ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.last_message_time_text_style.color);
        ImGui::Text("%s", time_str.c_str());
        ImGui::PopStyleColor();
    }

    // Render last message preview below name (replaces online/offline text status).
    float last_msg_y = name_y + m_current_style.user_name_text_style.font_size + 2;
    ImGui::SetCursorPos(ImVec2(name_x, last_msg_y));
    if (!item.last_message.empty()) {
        std::string msg = item.last_message;
        if (msg.length() > 30) msg = msg.substr(0, 27) + "..."; // truncate if too long.
        ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.last_message_text_style.color);
        ImGui::Text("%s", msg.c_str());
        ImGui::PopStyleColor();
    }

    if (!item.last_message.empty() && item.last_message_status != MessageStatus::Default) {
        std::string status_text = messageStatusToString(item.last_message_status);
        if (!status_text.empty()) {
            ImVec4 status_color;
            switch (item.last_message_status) {
            case MessageStatus::Sending:   status_color = ImVec4(0.8f, 0.8f, 0.0f, 1.0f); break; // желтый
            case MessageStatus::Sent:      status_color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f); break; // серый
            case MessageStatus::Delivered: status_color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f); break; // серый
            case MessageStatus::Read:      status_color = ImVec4(0.3f, 0.6f, 1.0f, 1.0f); break; // синий
            case MessageStatus::Failed:    status_color = ImVec4(1.0f, 0.2f, 0.2f, 1.0f); break; // красный
            default:                       status_color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f); break;
            }

            ImVec2 text_size = ImGui::CalcTextSize(status_text.c_str());
            float right_padding = m_current_style.user_block_style.padding;
            float bottom_padding = m_current_style.user_block_style.padding;

            ImGui::SetCursorPos(ImVec2(
                block_width - right_padding - text_size.x,
                block_height - bottom_padding - text_size.y
            ));

            ImGui::PushStyleColor(ImGuiCol_Text, status_color);
            ImGui::Text("%s", status_text.c_str());
            ImGui::PopStyleColor();
        }
    }

    ImGui::EndChild();
}

void ChatListWindow::renderUserAvatar(const User& user, float size)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList(); // Get draw list associated to current window , to append drawing primitives.

    // Find position of avatar circle center.
    ImVec2 cursor_screen = ImGui::GetCursorScreenPos();
    ImVec2 circle_center = ImVec2(cursor_screen.x + size / 2, cursor_screen.y + size / 2);
    float radius = size / 2 - m_current_style.avatar_style.border_width;

    // Convert ImVec4 to ImU32 for avatar color.
    ImU32 avatar_color = ImGui::ColorConvertFloat4ToU32(m_current_style.avatar_style.background_color);

    // Draw circle.
    draw_list->AddCircleFilled(circle_center, radius, avatar_color);

    // Draw circle border.
    ImU32 border_color = ImGui::ColorConvertFloat4ToU32(m_current_style.avatar_style.border_color);
    draw_list->AddCircle(circle_center, radius, border_color, 0, m_current_style.avatar_style.border_width);

    // Get initial.
    if (!user.display_name.empty()) {
        std::string initial(1, user.display_name[0]);

        // Draw initial at the circle center.
        ImVec2 text_size = ImGui::CalcTextSize(initial.c_str());
        ImVec2 text_pos = ImVec2(circle_center.x - text_size.x / 2,
            circle_center.y - text_size.y / 2);

        ImU32 text_color = ImGui::ColorConvertFloat4ToU32(m_current_style.avatar_style.text_color);
        draw_list->AddText(text_pos, text_color, initial.c_str());
    }

    // Reserve place for avatar.
    ImGui::Dummy(ImVec2(size, size));
}

std::string ChatListWindow::formatLastMessageTime(const std::chrono::system_clock::time_point& tp) const {
    using namespace std::chrono;
    if (tp == system_clock::time_point::min()) // No data.
        return "";

    system_clock::time_point now = system_clock::now();
    time_t msg_time_t = system_clock::to_time_t(tp);
    time_t now_time_t = system_clock::to_time_t(now);
    std::tm tm_msg, tm_now;
#ifdef _WIN32
    localtime_s(&tm_msg, &msg_time_t);
    localtime_s(&tm_now, &now_time_t);
#else
    localtime_r(&msg_time_t, &tm_msg);
    localtime_r(&now_time_t, &tm_now);
#endif

    // Today: hours:minutes.
    if (tm_msg.tm_year == tm_now.tm_year &&
        tm_msg.tm_mon == tm_now.tm_mon &&
        tm_msg.tm_mday == tm_now.tm_mday)
    {
        char buf[16];
        std::strftime(buf, sizeof(buf), "%H:%M", &tm_msg);
        return std::string(buf);
    }

    // Calculate difference in calendar days (local time).
    auto make_midnight = [](std::tm t) -> time_t {
        t.tm_hour = 0;
        t.tm_min = 0;
        t.tm_sec = 0;
        t.tm_isdst = -1;   // let mktime determine DST.
        return std::mktime(&t);
        };
    time_t msg_midnight = make_midnight(tm_msg);
    time_t now_midnight = make_midnight(tm_now);
    int diff_days = static_cast<int>((now_midnight - msg_midnight) / 86400);

    // Within the last 6 days (yesterday .. 6 days ago) => "Last week".
    if (diff_days > 0 && diff_days < 7) {
        return "Last week";
    }

    // Same year but older than 6 days => month name
    if (tm_msg.tm_year == tm_now.tm_year) {
        static const std::array<const char*, 12> months = {
            "Jan", "Feb", "Mar", "Apr",
            "May", "Jun", "Jul", "Aug",
            "Sept", "Oct", "Nov", "Dec"
        };
        return months[tm_msg.tm_mon];
    }

    // Previous year(s)
    int year_diff = tm_now.tm_year - tm_msg.tm_year;   // years since 1900
    if (year_diff == 1) {
        return "Last year";
    }
    else {   // year_diff >= 2
        return std::to_string(year_diff) + " years ago";
    }
}