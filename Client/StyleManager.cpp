#include "StyleManager.h"
#include "StyleManager.h"
#include "StyleManager.h"
#include "StyleManager.h"

StyleManager& StyleManager::getInstance()
{
    static StyleManager instance;
    return instance;
}

StyleManager::StyleManager() {
    setChatListWindowStyle();
    setChatWindowStyle();
}


const ChatListWindowStyle& StyleManager::getChatListWindowStyle() const
{
    return m_chat_list_window_style;
}

const ChatListWindowStyle& StyleManager::getChatWindowStyle() const
{
    return m_chat_window_style;
}

void StyleManager::setChatListWindowStyle()
{
    // Main text style for regular text.
    m_chat_list_window_style.main_text_style.color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    m_chat_list_window_style.main_text_style.font_size = 13.0f;
    m_chat_list_window_style.main_text_style.bold = false;
    m_chat_list_window_style.main_text_style.alignment = ImVec2(0.0f, 0.5f);

    // Refresh button.
    m_chat_list_window_style.refresh_button_text = "Refresh";
    m_chat_list_window_style.refresh_button_text_style.color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    m_chat_list_window_style.refresh_button_text_style.font_size = 13.0f;
    m_chat_list_window_style.refresh_button_text_style.bold = false;
    m_chat_list_window_style.refresh_button_text_style.alignment = ImVec2(0.5f, 0.5f);

    // Empty chat list text.
    m_chat_list_window_style.empty_chat_list_text = "Start chatting";
    m_chat_list_window_style.empty_chat_list_text_style.color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    m_chat_list_window_style.empty_chat_list_text_style.font_size = 13.0f;
    m_chat_list_window_style.empty_chat_list_text_style.bold = false;
    m_chat_list_window_style.empty_chat_list_text_style.alignment = ImVec2(0.5f, 0.5f);

    // User block style.
    m_chat_list_window_style.user_block_style.size = ImVec2(0, 40);  // 0 = auto width.
    m_chat_list_window_style.user_block_style.background = ImVec4(0.2f, 0.2f, 0.22f, 1.0f);  // IM_COL32(51, 51, 56, 255).
    m_chat_list_window_style.user_block_style.background_hover = ImVec4(0.25f, 0.25f, 0.27f, 1.0f);  // For hover effect.
    m_chat_list_window_style.user_block_style.border_radius = 0.0f;
    m_chat_list_window_style.user_block_style.border_width = 0.0f;
    m_chat_list_window_style.user_block_style.border_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    m_chat_list_window_style.user_block_style.shadow_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    m_chat_list_window_style.user_block_style.margin = 0.0f;
    m_chat_list_window_style.user_block_style.padding = 2.0f;

    // User name text style.
    m_chat_list_window_style.user_name_text_style.color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    m_chat_list_window_style.user_name_text_style.font_size = 14.0f;
    m_chat_list_window_style.user_name_text_style.bold = false;
    m_chat_list_window_style.user_name_text_style.alignment = ImVec2(0.0f, 0.0f);

    // User status text style.
    m_chat_list_window_style.user_status_text_style.color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    m_chat_list_window_style.user_status_text_style.font_size = 12.0f;
    m_chat_list_window_style.user_status_text_style.bold = false;
    m_chat_list_window_style.user_status_text_style.alignment = ImVec2(0.0f, 0.0f);

    // Last message preview style.
    m_chat_list_window_style.last_message_text_style.color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
    m_chat_list_window_style.last_message_text_style.font_size = 12.0f;
    m_chat_list_window_style.last_message_text_style.bold = false;
    m_chat_list_window_style.last_message_text_style.alignment = ImVec2(0.0f, 0.5f);

    // Time style.
    m_chat_list_style.last_message_time_text_style.color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    m_chat_list_style.last_message_time_text_style.font_size = 11.0f;
    m_chat_list_style.last_message_time_text_style.bold = false;
    m_chat_list_style.last_message_time_text_style.alignment = ImVec2(1.0f, 0.0f);

    // Avatar style.
    m_chat_list_window_style.avatar_style.size = 36.0f;
    m_chat_list_window_style.avatar_style.background_color = ImVec4(100.0f / 255.0f, 200.0f / 255.0f, 100.0f / 255.0f, 1.0f);  // IM_COL32(100, 200, 100, 255).
    m_chat_list_window_style.avatar_style.border_color = ImVec4(1.0f, 1.0f, 1.0f, 80.0f / 255.0f);  // IM_COL32(255, 255, 255, 80).
    m_chat_list_window_style.avatar_style.border_width = 1.5f;
    m_chat_list_window_style.avatar_style.text_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);  // IM_COL32(255, 255, 255, 255).

    // Refresh button colors.
    m_chat_list_window_style.refresh_button_color = ImVec4(0.27f, 0.51f, 0.78f, 1.0f);  // Standard imgui color.
    m_chat_list_window_style.refresh_button_hover_color = ImVec4(0.35f, 0.61f, 0.88f, 1.0f);  // Hover color.

    // Online indicator.
    m_chat_list_window_style.online_indicator_color = ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
    m_chat_list_window_style.online_indicator_radius = 4.0f;

    // Message sending status.
    m_chat_list_window_style.message_status_color[0] = ImVec4(0.8f, 0.8f, 0.0f, 1.0f); // Sending.     Yellow.
    m_chat_list_window_style.message_status_color[1] = ImVec4(0.7f, 0.7f, 0.7f, 1.0f); // Sent.        Gray.
    m_chat_list_window_style.message_status_color[2] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Delivered.   Whilte.
    m_chat_list_window_style.message_status_color[3] = ImVec4(0.7f, 0.7f, 0.7f, 1.0f); // Read.        Blue.
    m_chat_list_window_style.message_status_color[4] = ImVec4(1.0f, 0.2f, 0.2f, 1.0f); // Failed.      Red.
    m_chat_list_window_style.message_status_color[5] = ImVec4(0.7f, 0.7f, 0.7f, 1.0f); // Default.     Gray.

    // Offline status text.
    m_chat_list_window_style.offline_status_text = "Offline, Connecting...";
    m_chat_list_window_style.offline_status_text_style.color = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
    m_chat_list_window_style.offline_status_text_style.font_size = 13.0f;
    m_chat_list_window_style.offline_status_text_style.bold = false;
    m_chat_list_window_style.offline_status_text_style.alignment = ImVec2(0.5f, 0.5f);

    // Spacing.
    m_chat_list_window_style.spacing = 5.0f;

    // Tabs
    m_chat_list_window_style.chats_tab_text = "Chats";
    m_chat_list_window_style.users_tab_text = "Users";
    m_chat_list_window_style.tab_text_style.color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    m_chat_list_window_style.tab_text_style.font_size = 14.0f;
    m_chat_list_window_style.tab_text_style.bold = false;
    m_chat_list_window_style.tab_text_style.alignment = ImVec2(0.5f, 0.5f);
    m_chat_list_window_style.tab_button_color = ImVec4(0.2f, 0.2f, 0.22f, 1.0f);
    m_chat_list_window_style.tab_button_hover_color = ImVec4(0.3f, 0.3f, 0.33f, 1.0f);
    m_chat_list_window_style.tab_active_color = ImVec4(0.27f, 0.51f, 0.78f, 1.0f);

    // Search
    m_chat_list_window_style.search_field_hint = "Search chats, users...";
    m_chat_list_window_style.search_button_text = "Search";
    m_chat_list_window_style.search_hint_style.color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    m_chat_list_window_style.search_hint_style.font_size = 13.0f;
    m_chat_list_window_style.search_hint_style.bold = false;
    m_chat_list_window_style.search_hint_style.alignment = ImVec2(0.0f, 0.5f);
    m_chat_list_window_style.search_button_color = ImVec4(0.27f, 0.51f, 0.78f, 1.0f);
    m_chat_list_window_style.search_button_hover_color = ImVec4(0.35f, 0.61f, 0.88f, 1.0f);

    // Empty state — chats
    m_chat_list_window_style.no_chats_found_text = "No chats yet. Search for users to start messaging.";
    m_chat_list_window_style.no_chats_found_text_style.color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    m_chat_list_window_style.no_chats_found_text_style.font_size = 13.0f;
    m_chat_list_window_style.no_chats_found_text_style.bold = false;
    m_chat_list_window_style.no_chats_found_text_style.alignment = ImVec2(0.5f, 0.5f);

    // Empty state — search prompt
    m_chat_list_window_style.empty_search_prompt_text = "Type a username to search.";
    m_chat_list_window_style.empty_search_prompt_text_style.color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    m_chat_list_window_style.empty_search_prompt_text_style.font_size = 13.0f;
    m_chat_list_window_style.empty_search_prompt_text_style.bold = false;
    m_chat_list_window_style.empty_search_prompt_text_style.alignment = ImVec2(0.5f, 0.5f);

    // Empty state — no results
    m_chat_list_style.no_users_found_text = "No users found.";
    m_chat_list_style.no_users_found_text_style.color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    m_chat_list_style.no_users_found_text_style.font_size = 13.0f;
    m_chat_list_style.no_users_found_text_style.bold = false;
    m_chat_list_style.no_users_found_text_style.alignment = ImVec2(0.5f, 0.5f);

    // Empty state — search unavailable
    m_chat_list_window_style.search_unavailable_text = "Search unavailable while offline.";
    m_chat_list_window_style.search_unavailable_text_style.color = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
    m_chat_list_window_style.search_unavailable_text_style.font_size = 13.0f;
    m_chat_list_window_style.search_unavailable_text_style.bold = false;
    m_chat_list_window_style.search_unavailable_text_style.alignment = ImVec2(0.5f, 0.5f);
}

void StyleManager::setChatWindowStyle()
{
    m_chat_window_style.chat_header_text_style.color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    m_chat_window_style.chat_header_text_style.font_size = 16.0f;
    m_chat_window_style.chat_header_text_style.bold = true;
    m_chat_window_style.chat_header_text_style.alignment = ImVec2(0.0f, 0.5f);

    m_chat_window_style.empty_chat_text = "Select a contact to start chatting";
    m_chat_window_style.empty_chat_text_style.color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    m_chat_window_style.empty_chat_text_style.font_size = 14.0f;
    m_chat_window_style.empty_chat_text_style.bold = false;
    m_chat_window_style.empty_chat_text_style.alignment = ImVec2(0.5f, 0.5f);

    m_chat_window_style.input_hint_text = "Type a message...";
    m_chat_window_style.input_hint_text_style.color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    m_chat_window_style.input_hint_text_style.font_size = 13.0f;
    m_chat_window_style.input_hint_text_style.bold = false;
    m_chat_window_style.input_hint_text_style.alignment = ImVec2(0.0f, 0.5f);

    m_chat_window_style.send_button_text = "Send";
    m_chat_window_style.send_button_text_style.color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    m_chat_window_style.send_button_text_style.font_size = 13.0f;
    m_chat_window_style.send_button_text_style.bold = false;
    m_chat_window_style.send_button_text_style.alignment = ImVec2(0.5f, 0.5f);

    m_chat_window_style.send_button_color = ImVec4(0.27f, 0.51f, 0.78f, 1.0f);
    m_chat_window_style.send_button_hover_color = ImVec4(0.35f, 0.61f, 0.88f, 1.0f);
    m_chat_window_style.send_button_active_color = ImVec4(0.2f, 0.4f, 0.6f, 1.0f);

    m_chat_window_style.input_background_color = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    m_chat_window_style.chat_background_color = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    m_chat_window_style.separator_color = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);

    // Message bubble
    m_chat_window_style.message_style.sender_bg_color = ImVec4(0.2f, 0.6f, 0.2f, 0.3f);
    m_chat_window_style.message_style.reciever_bg_color = ImVec4(0.3f, 0.3f, 0.3f, 0.3f);
    m_chat_window_style.message_style.text_style.color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    m_chat_window_style.message_style.text_style.font_size = 14.0f;
    m_chat_window_style.message_style.text_style.bold = false;
    m_chat_window_style.message_style.text_style.alignment = ImVec2(0.0f, 0.0f);

    m_chat_window_style.message_style.time_text_style.color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
    m_chat_window_style.message_style.time_text_style.font_size = 11.0f;
    m_chat_window_style.message_style.time_text_style.bold = false;
    m_chat_window_style.message_style.time_text_style.alignment = ImVec2(1.0f, 1.0f);

    // Status colors (Sending, Sent, Delivered, Read, Failed, Default)
    m_chat_window_style.message_style.status_style[0].color = ImVec4(0.8f, 0.8f, 0.0f, 1.0f); // Sending - yellow
    m_chat_window_style.message_style.status_style[1].color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f); // Sent - gray
    m_chat_window_style.message_style.status_style[2].color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Delivered - white
    m_chat_window_style.message_style.status_style[3].color = ImVec4(0.4f, 0.5f, 0.9f, 1.0f); // Read - blue
    m_chat_window_style.message_style.status_style[4].color = ImVec4(1.0f, 0.2f, 0.2f, 1.0f); // Failed - red
    m_chat_window_style.message_style.status_style[5].color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f); // Default - gray

    for (TextStyle& s : m_chat_window_style.message_style.status_style) {
        s.font_size = 11.0f;
        s.bold = false;
        s.alignment = ImVec2(1.0f, 1.0f);
    }

    m_chat_window_style.message_style.bubble_padding = 4.0f;
    m_chat_window_style.message_style.bubble_margin = 2.0f;
    m_chat_window_style.message_style.max_text_width_ratio = 0.7f;
    m_chat_window_style.message_style.border_color = ImVec4(0.5f, 0.5f, 0.5f, 0.5f);
    m_chat_window_style.message_style.border_width = 1.0f;
}




