#pragma once
#include "imgui.h"
#include <string>

struct BlockStyle {
    ImVec2 size;
    ImVec4 background;
    ImVec4 background_hover;
    float border_radius;
    float border_width;
    ImVec4 border_color;
    ImVec4 shadow_color;
    float margin;
    float padding;
};

struct TextStyle {
    ImVec4 color;
    float font_size;
    bool bold;
    ImVec2 alignment; // 0-1 for x and y
};

struct AvatarStyle {
    float size;
    ImVec4 background_color;
    ImVec4 border_color;
    float border_width;
    ImVec4 text_color;
};

struct ChatListWindowStyle {
    TextStyle main_text_style;
    std::string refresh_button_text;
    TextStyle refresh_button_text_style;
    std::string empty_chat_list_text;
    TextStyle empty_chat_list_text_style;
    BlockStyle user_block_style;
    TextStyle user_name_text_style;
    TextStyle user_status_text_style;
    TextStyle last_message_text_style;
    TextStyle last_message_time_text_style;
    AvatarStyle avatar_style;
    ImVec4 refresh_button_color;
    ImVec4 refresh_button_hover_color;
    ImVec4 online_indicator_color;
    float online_indicator_radius;
    float spacing;
};

/**
 * @brief Singleton style manager.
 */
class StyleManager {
public:
    static StyleManager& getInstance();

    ChatListWindowStyle getChatListWindowStyle() const;

private:
    StyleManager() = default;
    ~StyleManager() = default;
    StyleManager(const StyleManager&) = delete;
    StyleManager& operator=(const StyleManager&) = delete;
};