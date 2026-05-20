#include "StyleManager.h"

StyleManager& StyleManager::getInstance()
{
    static StyleManager instance;
    return instance;
}

ChatListWindowStyle StyleManager::getChatListWindowStyle() const
{
    ChatListWindowStyle style;

    // Main text style for regular text.
    style.main_text_style.color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.main_text_style.font_size = 13.0f;
    style.main_text_style.bold = false;
    style.main_text_style.alignment = ImVec2(0.0f, 0.5f);

    // Refresh button.
    style.refresh_button_text = "Refresh";
    style.refresh_button_text_style.color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.refresh_button_text_style.font_size = 13.0f;
    style.refresh_button_text_style.bold = false;
    style.refresh_button_text_style.alignment = ImVec2(0.5f, 0.5f);
    
    // Empty chat list text.
    style.empty_chat_list_text = "Start chatting";
    style.empty_chat_list_text_style.color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    style.empty_chat_list_text_style.font_size = 13.0f;
    style.empty_chat_list_text_style.bold = false;
    style.empty_chat_list_text_style.alignment = ImVec2(0.5f, 0.5f);

    // User block style.
    style.user_block_style.size = ImVec2(0, 40);  // 0 = auto width.
    style.user_block_style.background = ImVec4(0.2f, 0.2f, 0.22f, 1.0f);  // IM_COL32(51, 51, 56, 255).
    style.user_block_style.background_hover = ImVec4(0.25f, 0.25f, 0.27f, 1.0f);  // For hover effect.
    style.user_block_style.border_radius = 0.0f;
    style.user_block_style.border_width = 0.0f;
    style.user_block_style.border_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.user_block_style.shadow_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.user_block_style.margin = 0.0f;
    style.user_block_style.padding = 2.0f;

    // User name text style.
    style.user_name_text_style.color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.user_name_text_style.font_size = 14.0f;
    style.user_name_text_style.bold = false;
    style.user_name_text_style.alignment = ImVec2(0.0f, 0.0f);

    // User status text style.
    style.user_status_text_style.color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    style.user_status_text_style.font_size = 12.0f;
    style.user_status_text_style.bold = false;
    style.user_status_text_style.alignment = ImVec2(0.0f, 0.0f);

    // Last message preview style
    style.last_message_text_style.color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
    style.last_message_text_style.font_size = 12.0f;
    style.last_message_text_style.bold = false;
    style.last_message_text_style.alignment = ImVec2(0.0f, 0.5f);

    // Time style
    style.last_message_time_text_style.color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    style.last_message_time_text_style.font_size = 11.0f;
    style.last_message_time_text_style.bold = false;
    style.last_message_time_text_style.alignment = ImVec2(1.0f, 0.0f);

    // Avatar style.
    style.avatar_style.size = 36.0f;
    style.avatar_style.background_color = ImVec4(100.0f / 255.0f, 200.0f / 255.0f, 100.0f / 255.0f, 1.0f);  // IM_COL32(100, 200, 100, 255).
    style.avatar_style.border_color = ImVec4(1.0f, 1.0f, 1.0f, 80.0f / 255.0f);  // IM_COL32(255, 255, 255, 80).
    style.avatar_style.border_width = 1.5f;
    style.avatar_style.text_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);  // IM_COL32(255, 255, 255, 255).

    // Refresh button colors.
    style.refresh_button_color = ImVec4(0.27f, 0.51f, 0.78f, 1.0f);  // Standard imgui color.
    style.refresh_button_hover_color = ImVec4(0.35f, 0.61f, 0.88f, 1.0f);  // Hower color.

    // Online indicator
    style.online_indicator_color = ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
    style.online_indicator_radius = 4.0f;

    // Spacing.
    style.spacing = 5.0f;

    return style;
}

