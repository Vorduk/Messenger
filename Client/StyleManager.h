#pragma once
#include "imgui.h"
#include <string>
#include <array>

/**
 * @brief Visual layout and appearance for a rectangular UI block.
 * Contains size, background colors, border and shadow parameters,
 * and spacing values. 
 */
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

/**
 * @brief Text rendering options.
 * Includes color, font size, weight (bold) and alignment (0..1 for x/y). 
 */
struct TextStyle {
    ImVec4 color;
    float font_size;
    bool bold;
    ImVec2 alignment;
};

/**
 * @brief Appearance settings for a circular/avatar element.
 * Defines diameter, background and border colors, border width and text color. 
 */
struct AvatarStyle {
    float size;
    ImVec4 background_color;
    ImVec4 border_color;
    float border_width;
    ImVec4 text_color;
};

/**
 * @brief All style settings used by the chat list window*
 */
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
    std::array<ImVec4, 6> message_status_color;
    TextStyle offline_status_text_style;
    std::string offline_status_text;
    float online_indicator_radius;
    float spacing;
    std::string chats_tab_text;
    std::string users_tab_text;
    TextStyle tab_text_style;
    ImVec4 tab_button_color;
    ImVec4 tab_button_hover_color;
    ImVec4 tab_active_color;
    std::string search_field_hint;
    std::string search_button_text;
    TextStyle search_hint_style;
    ImVec4 search_button_color;
    ImVec4 search_button_hover_color;
    std::string no_chats_found_text;
    TextStyle no_chats_found_text_style;
    std::string empty_search_prompt_text;
    TextStyle empty_search_prompt_text_style;
    std::string no_users_found_text;       
    TextStyle no_users_found_text_style;
    std::string search_unavailable_text;
    TextStyle search_unavailable_text_style;
};

/**
 * @brief Singleton manager that provides UI styles.
 * 
 * getChatListWindowStyle() and other style getters: The returned reference refers to the manager's
 * internal ChatListWindowStyle; modifying the manager's style (via its
 * setters) will immediately affect any client windows holding a reference to it.
 */
class StyleManager {
public:
    static StyleManager& getInstance();

    // Get style reference for particular window.
    const ChatListWindowStyle& getChatListWindowStyle() const;
    //const ChatListWindowStyle& getLoginWindowStyle() const; // todo
    //const ChatListWindowStyle& getChatWindowStyle() const; // todo

private:
    StyleManager();
    ~StyleManager() = default;
    StyleManager(const StyleManager&) = delete;
    StyleManager& operator=(const StyleManager&) = delete;

    // Fill the windows style structures.
    void setChatListWindowStyle();
    // void setLoginWindowStyle(); // todo
    // void setChatWindowStyle(); // todo

    // All windows styles stored here.
    ChatListWindowStyle m_chat_list_style;
};