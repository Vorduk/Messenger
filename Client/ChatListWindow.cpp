#include "ChatListWindow.h"
#include "imgui.h"
#include "LogMacros.h"

ChatListWindow::ChatListWindow(GetUsersUseCase& get_users_uc, const std::string& current_user_id)
    : m_get_users_uc(get_users_uc), m_current_user_id(current_user_id) {

    m_current_style = StyleManager::getInstance().getChatListWindowStyle();
    refreshUsers();

}

void ChatListWindow::setOnUserSelected(std::function<void(const User&)> callback) {
    m_on_selected = std::move(callback);
}

void ChatListWindow::refreshUsers() {
    m_get_users_uc.execute(m_current_user_id,
        [this](std::vector<User> users) {
            m_users = std::move(users);
            LOG_INFO("User list updated, count: {}", m_users.size());
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

    if (m_users.empty()) {
        // Use empty chat list text and style from style
        ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.empty_chat_list_text_style.color);
        ImGui::Text("%s", m_current_style.empty_chat_list_text.c_str());
        ImGui::PopStyleColor();
    }
    else {
        // Render all users elements.
        for (size_t i = 0; i < m_users.size(); ++i) {
            const User& user = m_users[i];

            renderUserBlock(user);

            ImGui::Spacing();
        }
    }
}

void ChatListWindow::renderUserBlock(const User& user)
{
    std::string block_id = "user_block_" + user.id; // Unique name for imgui.

    ImVec2 block_start = ImGui::GetCursorScreenPos();
    float block_width = ImGui::GetContentRegionAvail().x;

    // Use block size from style (height from style, width auto)
    float block_height = m_current_style.user_block_style.size.y;
    ImGui::BeginChild(block_id.c_str(), ImVec2(block_width, block_height), false, ImGuiWindowFlags_NoScrollbar);

    // Use padding from style for avatar position
    ImGui::SetCursorPos(ImVec2(m_current_style.user_block_style.padding, m_current_style.user_block_style.padding));
    renderUserAvatar(user, m_current_style.avatar_style.size);

    // Calculate name position based on avatar size and padding
    float name_x = m_current_style.avatar_style.size + m_current_style.user_block_style.padding * 2;
    float name_y = m_current_style.user_block_style.padding;
    ImGui::SetCursorPos(ImVec2(name_x, name_y));

    // Render name with style
    std::string display_name = user.display_name.empty() ? user.username : user.display_name;
    ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.user_name_text_style.color);
    ImGui::Text("%s", display_name.c_str());
    ImGui::PopStyleColor();

    // Calculate status position (below name)
    float status_y = name_y + m_current_style.user_name_text_style.font_size + 2;
    ImGui::SetCursorPos(ImVec2(name_x, status_y));

    // Render online/offline status with style
    ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.user_status_text_style.color);
    if (user.is_online) {
        // Keep original online color (green), but use status text style as base
        ImVec4 online_color = ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
        ImGui::TextColored(online_color, "Online");
    }
    else {
        ImGui::Text("Offline");
    }
    ImGui::PopStyleColor();

    ImGui::EndChild();
}

void ChatListWindow::renderUserAvatar(const User& user, float size)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList(); // Get draw list associated to current window , to append drawing primitives.

    // Find position of avatar circle center
    ImVec2 cursor_screen = ImGui::GetCursorScreenPos();
    ImVec2 circle_center = ImVec2(cursor_screen.x + size / 2, cursor_screen.y + size / 2);
    float radius = size / 2 - m_current_style.avatar_style.border_width;

    // Convert ImVec4 to ImU32 for avatar color
    ImU32 avatar_color = ImGui::ColorConvertFloat4ToU32(m_current_style.avatar_style.background_color);

    // Draw circle.
    draw_list->AddCircleFilled(circle_center, radius, avatar_color);

    // Draw circle border.
    ImU32 border_color = ImGui::ColorConvertFloat4ToU32(m_current_style.avatar_style.border_color);
    draw_list->AddCircle(circle_center, radius, border_color, 0, m_current_style.avatar_style.border_width);

    // Get initial
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
