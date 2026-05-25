#include "ChatListWindow.h"
#include "imgui.h"
#include "LogMacros.h"

ChatListWindow::ChatListWindow(const ChatListWindowStyle& current_style, GetUsersUseCase& get_users_uc, GetChatsUseCase& get_chats_uc, IMessageRepository& local_repo, const std::string& current_user_id)
    : m_current_style(current_style)
    , m_get_users_uc(get_users_uc)
    , m_get_chats_uc(get_chats_uc)
    , m_local_repo(local_repo)
    , m_current_user_id(current_user_id)
{
    loadChats(); // Load existing chats on startup (from server or cache)
}

const char* ChatListWindow::getName() const
{
    return "Chat List";
}

void ChatListWindow::setOnUserSelected(std::function<void(const User&)> callback) {
    m_on_selected = std::move(callback); // Will be called from renderUserBloack() if button pressed.
}

void ChatListWindow::refreshUsers() {
    // Called from outside, e.g. after reconnection or manual refresh

    m_last_chats_refresh = 0.0;

    loadChats();
}

void ChatListWindow::loadChats() {
    m_get_chats_uc.execute(m_current_user_id, [this](bool success, std::vector<ChatListItem> items) {
        if (success) {
            // Сервер ответил, даже если список пуст
            m_is_offline = false;
            m_chats = std::move(items);
            m_local_repo.cacheChatList(m_current_user_id, m_chats); // кешируем в том числе пустой список
            LOG_INFO("Chats loaded from server, count: {}", m_chats.size());
        }
        else {
            // Сетевая ошибка — пробуем загрузить из кеша
            std::vector<ChatListItem> cached = m_local_repo.getCachedChatList(m_current_user_id);
            if (!cached.empty()) {
                m_chats = std::move(cached);
                m_is_offline = true;
                LOG_INFO("Chats loaded from local cache, count: {}", m_chats.size());
            }
            else {
                m_chats.clear();
                m_is_offline = true;  // действительно офлайн и кеша нет
            }
        }
        });
}

// Update last message in chat item of this class.
void ChatListWindow::updateLastMessage(const std::string& partner_id, const std::string& text, const std::chrono::system_clock::time_point& timestamp, MessageStatus message_status)
{
    // Find the chat with the given partner_id
    std::vector<ChatListItem>::iterator it = std::find_if(m_chats.begin(), m_chats.end(),
        [&partner_id](const ChatListItem& chat) {
            return chat.user.id == partner_id;
        }
    );

    // If not found, nothing to update
    if (it == m_chats.end()) {
        return;
    }

    // Update message fields
    it->last_message = text;
    it->last_message_time = timestamp;
    it->last_message_status = message_status;

    // Move the updated chat to the front of the list if it's not already there
    if (it != m_chats.begin()) {
        // std::rotate shifts the range [first, it) forward and places
        // the element at 'it' to the position 'first'.
        // Rotate the range [m_chats.begin(), it + 1) left by one position.
        std::rotate(m_chats.begin(), it, std::next(it));
    }

    m_local_repo.cacheChatList(m_current_user_id, m_chats);
}

void ChatListWindow::searchUsers(const std::string& query) {
    if (query.empty()) {
        m_search_results.clear();
        m_is_user_search_failed = false;
        return;
    }

    m_get_users_uc.search(m_current_user_id, query,
        [this](bool success, std::vector<ChatListItem> items) {
            if (success) {
                m_search_results = std::move(items);
                m_is_user_search_failed = false;
                LOG_INFO("User search results updated, count: {}", m_search_results.size());
            }
            else {
                m_is_user_search_failed = true;
                LOG_WARN("User search failed - server unreachable");
            }
        }
    );
}

// Rendering helpers.

void ChatListWindow::render() {

    // Update each 8 seconds.
    double now = ImGui::GetTime();
    if (!m_is_offline && now - m_last_chats_refresh > 8.0) {
        m_last_chats_refresh = now;
        loadChats();
    }

    renderSearchBar();
    renderTabs();

    ImGui::Separator();

    // Show offline indicator when using cached data
    if (m_is_offline) {
        ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.offline_status_text_style.color);
        ImGui::Text("%s", m_current_style.offline_status_text.c_str());
        ImGui::PopStyleColor();
    }

    if (m_active_tab == Tab::Chats) {
        renderChatList();
    }
    else {
        renderUserList();
    }
}

void ChatListWindow::renderSearchBar() {
    // Style for hint text.
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, m_current_style.search_hint_style.color);

    // Search input.
    float button_width = ImGui::CalcTextSize(m_current_style.search_button_text.c_str()).x +
        ImGui::GetStyle().FramePadding.x * 2;
    ImGui::PushItemWidth(-button_width - 10);
    ImGui::InputTextWithHint("##SearchField",
        m_current_style.search_field_hint.c_str(),
        m_search_buffer,
        IM_ARRAYSIZE(m_search_buffer));
    ImGui::PopItemWidth();

    ImGui::PopStyleColor();  // TextDisabled.

    ImGui::SameLine();

    // Search button.
    ImGui::PushStyleColor(ImGuiCol_Button, m_current_style.search_button_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, m_current_style.search_button_hover_color);
    ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.tab_text_style.color);

    if (ImGui::Button(m_current_style.search_button_text.c_str())) {
        if (m_active_tab == Tab::Chats) {
            // Local filter. // todo.
        }
        else {
            searchUsers(m_search_buffer);
        }
    }

    ImGui::PopStyleColor(3);
}

void ChatListWindow::renderTabs() {
    ImGui::PushStyleColor(ImGuiCol_Button, m_current_style.tab_button_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, m_current_style.tab_button_hover_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, m_current_style.tab_active_color);
    ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.tab_text_style.color);

    // Light active tab.
    if (m_active_tab == Tab::Chats) {
        ImGui::PushStyleColor(ImGuiCol_Button, m_current_style.tab_active_color);
        if (ImGui::Button(m_current_style.chats_tab_text.c_str())) {
        }
        ImGui::PopStyleColor();
    }
    else {
        if (ImGui::Button(m_current_style.chats_tab_text.c_str())) {
            m_active_tab = Tab::Chats;
        }
    }

    ImGui::SameLine();

    if (m_active_tab == Tab::Users) {
        ImGui::PushStyleColor(ImGuiCol_Button, m_current_style.tab_active_color);
        if (ImGui::Button(m_current_style.users_tab_text.c_str())) {
        }
        ImGui::PopStyleColor();
    }
    else {
        if (ImGui::Button(m_current_style.users_tab_text.c_str())) {
            m_active_tab = Tab::Users;
        }
    }

    ImGui::PopStyleColor(4);  // Button, ButtonHovered, ButtonActive, Text
}

void ChatListWindow::renderChatList() {
    if (m_chats.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.no_chats_found_text_style.color);
        ImGui::Text("%s", m_current_style.no_chats_found_text.c_str());
        ImGui::PopStyleColor();
    }
    else {
        for (const ChatListItem& chat : m_chats) {
            renderUserBlock(chat);
            ImGui::Spacing();
        }
    }
}

void ChatListWindow::renderUserList() {
    if (m_search_results.empty() && strlen(m_search_buffer) == 0) {
        // Empty search query
        ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.empty_search_prompt_text_style.color);
        ImGui::Text("%s", m_current_style.empty_search_prompt_text.c_str());
        ImGui::PopStyleColor();
    }
    else if (m_search_results.empty() && m_is_user_search_failed) {
        // Server is unavailable — показываем предупреждение
        ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.search_unavailable_text_style.color);
        ImGui::Text("%s", m_current_style.search_unavailable_text.c_str());
        ImGui::PopStyleColor();
    }
    else if (m_search_results.empty()) {
        // Search complete, nothing found.
        ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.no_users_found_text_style.color);
        ImGui::Text("%s", m_current_style.no_users_found_text.c_str());
        ImGui::PopStyleColor();
    }
    else {
        for (const ChatListItem& item : m_search_results) {
            renderUserBlock(item);
            ImGui::Spacing();
        }
    }
}

void ChatListWindow::renderUserBlock(const ChatListItem& chat)
{
    const User& user = chat.user;
    std::string block_id = "user_block_" + user.id; // Unique name for imgui.

    float block_width = ImGui::GetContentRegionAvail().x;

    // Use block size from style (height from style, width auto).
    float block_height = m_current_style.user_block_style.size.y;
    ImGui::BeginChild(block_id.c_str(), ImVec2(block_width, block_height), false, ImGuiWindowFlags_NoScrollbar);

    // Invisible button for user selection
    ImGui::SetCursorPos(ImVec2(0, 0));
    if (ImGui::InvisibleButton(("select_" + user.id).c_str(), ImVec2(block_width, block_height))) {
        if (m_on_selected) {
            m_on_selected(user);
        }
    }

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Hover style.
    if (ImGui::IsItemHovered()) {
        ImVec2 win_pos = ImGui::GetWindowPos();
        ImVec2 win_size = ImGui::GetWindowSize();
        draw_list->AddRectFilled(
            win_pos,
            ImVec2(win_pos.x + win_size.x, win_pos.y + win_size.y),
            ImGui::ColorConvertFloat4ToU32(m_current_style.user_block_style.background_hover)
        );
    }

    // Use padding from style for avatar position.
    ImGui::SetCursorPos(ImVec2(m_current_style.user_block_style.padding, m_current_style.user_block_style.padding));
    ImVec2 avatar_cursor = ImGui::GetCursorScreenPos(); // remember screen position for online indicator.
    renderUserAvatar(user, m_current_style.avatar_style.size);

    // Draw online indicator (green circle) at bottom-right of avatar.
    if (user.is_online) {
        draw_list = ImGui::GetWindowDrawList();
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
    std::string time_str = formatLastMessageTime(chat.last_message_time);
    if (!time_str.empty()) {
        ImGui::SameLine();
        float time_width = ImGui::CalcTextSize(time_str.c_str()).x;
        float right_padding = m_current_style.user_block_style.padding;
        ImGui::SetCursorPosX(block_width - right_padding - time_width);
        ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.last_message_time_text_style.color);
        ImGui::Text("%s", time_str.c_str());
        ImGui::PopStyleColor();
    }

    // Render last message preview below name.
    float last_msg_y = name_y + m_current_style.user_name_text_style.font_size + 2;
    ImGui::SetCursorPos(ImVec2(name_x, last_msg_y));
    if (!chat.last_message.empty()) {
        std::string msg = chat.last_message;
        if (msg.length() > 30) msg = msg.substr(0, 27) + "..."; // truncate if too long.
        ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.last_message_text_style.color);
        ImGui::Text("%s", msg.c_str());
        ImGui::PopStyleColor();
    }

    // Render message status indicator at bottom-right of the block
    if (!chat.last_message.empty() && chat.last_message_status != MessageStatus::Default) {
        std::string status_text = messageStatusToString(chat.last_message_status);
        if (!status_text.empty()) {
            ImVec4 status_color;
            switch (chat.last_message_status) {
                case MessageStatus::Sending:   status_color = m_current_style.message_status_color[0]; break;
                case MessageStatus::Sent:      status_color = m_current_style.message_status_color[1]; break;
                case MessageStatus::Delivered: status_color = m_current_style.message_status_color[2]; break;
                case MessageStatus::Read:      status_color = m_current_style.message_status_color[3]; break;
                case MessageStatus::Failed:    status_color = m_current_style.message_status_color[4]; break;
                default:                       status_color = m_current_style.message_status_color[5]; break;
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
    ImDrawList* draw_list = ImGui::GetWindowDrawList(); // Get draw list associated to current window, to append drawing primitives.

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

// Message time format helper.
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
    std::function<time_t(std::tm)> make_midnight = [](std::tm t) -> time_t {
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
