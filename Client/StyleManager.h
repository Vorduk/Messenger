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
 * @brief All style settings used by the chat list window.
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
 * @brief Style settings for message block.
 */
struct MessageStyle {
    ImVec4 sender_bg_color;
    ImVec4 reciever_bg_color;
    TextStyle text_style;
    TextStyle time_text_style;
    TextStyle status_style[6]; 
    float bubble_padding;      
    float bubble_margin;       
    float max_text_width_ratio;
    ImVec4 border_color;
    float border_width;
    float bubble_rounding;
};

/**
 * @brief All style settings used by the chat window.
 */
struct ChatWindowStyle {
    TextStyle chat_header_text_style;
    std::string empty_chat_text;
    TextStyle empty_chat_text_style;
    std::string input_hint_text;
    TextStyle input_hint_text_style;
    std::string send_button_text;
    TextStyle send_button_text_style;
    ImVec4 send_button_color;
    ImVec4 send_button_hover_color;
    ImVec4 send_button_active_color;
    ImVec4 input_background_color;
    ImVec4 chat_background_color;
    ImVec4 separator_color;
    MessageStyle message_style;
    std::string chat_header_prefix;
};

/**
 * @brief All style settings used by the login/register window.
 */
struct LoginWindowStyle {
    // Common
    TextStyle title_text_style;      
    ImVec4 window_bg_color;          
    float fields_max_width;          

    // Tabs
    std::string login_tab_text;      
    std::string register_tab_text;   
    TextStyle tab_text_style;        
    ImVec4 tab_button_color;         
    ImVec4 tab_button_hover_color;   
    ImVec4 tab_active_color;         

    // Input fields
    float input_rounding;            
    float input_frame_padding;       
    ImVec4 input_bg_color;           
    ImVec4 input_border_color;       
    float input_border_width;        
    TextStyle hint_text_style;       
    TextStyle input_text_style;      

    // Buttons
    std::string login_button_text;   
    std::string register_button_text;
    TextStyle button_text_style;     
    ImVec4 button_color;             
    ImVec4 button_hover_color;       
    ImVec4 button_active_color;      
    float button_rounding;           

    // Error message
    TextStyle error_text_style;      
    ImVec4 error_text_color;         

    // Waiting state
    std::string waiting_text;        
    TextStyle waiting_text_style;    

    // Validation error strings
    std::string error_username_empty_text;    
    std::string error_username_invalid_text;
    std::string error_display_name_empty_text;
    std::string error_birthday_empty_text;
    std::string error_birthday_invalid_text;
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
    const ChatWindowStyle& getChatWindowStyle() const;
    const LoginWindowStyle& getLoginWindowStyle() const;
    
private:
    StyleManager();
    ~StyleManager() = default;
    StyleManager(const StyleManager&) = delete;
    StyleManager& operator=(const StyleManager&) = delete;

    // Fill the windows style structures.
    void setChatListWindowStyle();
    void setChatWindowStyle();
    void setLoginWindowStyle();

    // All windows styles stored here.
    ChatListWindowStyle m_chat_list_window_style;
    ChatWindowStyle m_chat_window_style;
    LoginWindowStyle m_login_window_style;
};