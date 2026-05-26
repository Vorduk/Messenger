#include "LoginWindow.h"
#include "imgui.h"
#include <cstring>
#include <ctime>
#include <algorithm>

// Constructor

LoginWindow::LoginWindow(const LoginWindowStyle& current_style,
    LoginUseCase& login_uc, RegisterUseCase& register_uc,
    LoginCallback on_login)
    : m_style(current_style)
    , m_login_uc(login_uc)
    , m_register_uc(register_uc)
    , m_on_login(std::move(on_login))
{
    memset(m_username, 0, sizeof(m_username));
    memset(m_display_name, 0, sizeof(m_display_name));
}

// Render

void LoginWindow::render()
{
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float form_height = (m_active_tab == Tab::Login) ? 150.0f : 260.0f;
    float form_width = m_style.fields_max_width;

    ImGui::SetCursorPosX((avail.x - form_width) * 0.5f);
    ImGui::SetCursorPosY((avail.y - form_height) * 0.5f);

    // Remove borders of the main window and the inner child window.
    // Use the window's background colour for the border so it blends in completely.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, m_style.window_bg_color);

    bool child_visible = ImGui::BeginChild("LoginForm", ImVec2(form_width, form_height), false,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);

    if (!child_visible) {
        // If the child window can't be created, undo the style pushes before leaving.
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
        return;
    }

    // Title
    ImGui::PushStyleColor(ImGuiCol_Text, m_style.title_text_style.color);
    ImGui::Text("Welcome to Messenger!");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    if (m_is_waiting) {
        ImGui::PushStyleColor(ImGuiCol_Text, m_style.waiting_text_style.color);
        ImGui::Text("%s", m_style.waiting_text.c_str());
        ImGui::PopStyleColor();
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
        return;
    }

    renderTabs();

    if (m_active_tab == Tab::Login) {
        renderLoginTab();
    }
    else {
        renderRegisterTab();
    }

    if (!m_error_message.empty()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, m_style.error_text_color);
        ImGui::TextWrapped("%s", m_error_message.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::EndChild();

    // Restore the original styles.
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

// Tabs

void LoginWindow::renderTabs()
{
    ImGui::PushStyleColor(ImGuiCol_Button,        m_style.tab_button_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  m_style.tab_button_hover_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   m_style.tab_active_color);
    ImGui::PushStyleColor(ImGuiCol_Text,           m_style.tab_text_style.color);

    float tab_width = (m_style.fields_max_width - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

    // Login tab
    bool login_was_active = (m_active_tab == Tab::Login);
    if (login_was_active)
        ImGui::PushStyleColor(ImGuiCol_Button, m_style.tab_active_color);

    if (ImGui::Button(m_style.login_tab_text.c_str(), ImVec2(tab_width, 0))) {
        m_active_tab = Tab::Login;
        m_error_message.clear();
    }

    if (login_was_active)
        ImGui::PopStyleColor();

    ImGui::SameLine();

    // Register tab
    bool register_was_active = (m_active_tab == Tab::Register);
    if (register_was_active)
        ImGui::PushStyleColor(ImGuiCol_Button, m_style.tab_active_color);

    if (ImGui::Button(m_style.register_tab_text.c_str(), ImVec2(tab_width, 0))) {
        m_active_tab = Tab::Register;
        m_error_message.clear();
    }

    if (register_was_active)
        ImGui::PopStyleColor();

    ImGui::PopStyleColor(4);
    ImGui::Spacing();
}

// Tab content

void LoginWindow::renderLoginTab()
{
    renderUsernameField();
    ImGui::Spacing();
    renderActionButton();   // "Login"
}

void LoginWindow::renderRegisterTab()
{
    renderUsernameField();
    ImGui::Spacing();
    renderDisplayNameField();
    ImGui::Spacing();
    renderBirthdayFields();
    ImGui::Spacing();
    renderActionButton();   // "Register"
}

// Input fields

void LoginWindow::renderUsernameField()
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,  m_style.input_rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,   ImVec2(m_style.input_frame_padding, m_style.input_frame_padding));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,            m_style.input_bg_color);
    ImGui::PushStyleColor(ImGuiCol_Border,             m_style.input_border_color);
    ImGui::PushStyleColor(ImGuiCol_Text,               m_style.input_text_style.color);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled,       m_style.hint_text_style.color);

    ImGui::PushItemWidth(m_style.fields_max_width);
    ImGui::InputTextWithHint("##Username", "Username", m_username, IM_ARRAYSIZE(m_username));
    ImGui::PopItemWidth();

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
}

void LoginWindow::renderDisplayNameField()
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,  m_style.input_rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,   ImVec2(m_style.input_frame_padding, m_style.input_frame_padding));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,            m_style.input_bg_color);
    ImGui::PushStyleColor(ImGuiCol_Border,             m_style.input_border_color);
    ImGui::PushStyleColor(ImGuiCol_Text,               m_style.input_text_style.color);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled,       m_style.hint_text_style.color);

    ImGui::PushItemWidth(m_style.fields_max_width);
    ImGui::InputTextWithHint("##DisplayName", "Display Name", m_display_name, IM_ARRAYSIZE(m_display_name));
    ImGui::PopItemWidth();

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
}

void LoginWindow::renderBirthdayFields()
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,  m_style.input_rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,   ImVec2(m_style.input_frame_padding, m_style.input_frame_padding));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,            m_style.input_bg_color);
    ImGui::PushStyleColor(ImGuiCol_Border,             m_style.input_border_color);
    ImGui::PushStyleColor(ImGuiCol_Text,               m_style.input_text_style.color);

    float total_width = m_style.fields_max_width;
    float spacing     = ImGui::GetStyle().ItemSpacing.x;
    float field_width = (total_width - 2.0f * spacing) / 3.0f;

    // Column labels
    ImGui::PushStyleColor(ImGuiCol_Text, m_style.hint_text_style.color);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX());
    ImGui::Text("Year");   ImGui::SameLine(ImGui::GetCursorPosX() + field_width + spacing);
    ImGui::Text("Month");  ImGui::SameLine(ImGui::GetCursorPosX() + field_width + spacing);
    ImGui::Text("Day");
    ImGui::PopStyleColor();

    // Input fields
    ImGui::PushItemWidth(field_width);
    ImGui::InputInt("##Year",  &m_birthday_year,  0, 0);
    ImGui::SameLine();
    ImGui::InputInt("##Month", &m_birthday_month, 0, 0);
    ImGui::SameLine();
    ImGui::InputInt("##Day",   &m_birthday_day,   0, 0);
    ImGui::PopItemWidth();

    ImGui::PopStyleColor(3);  // FrameBg, Border, Text
    ImGui::PopStyleVar(2);    // Rounding, Padding
}

// Action button

void LoginWindow::renderActionButton()
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, m_style.button_rounding);
    ImGui::PushStyleColor(ImGuiCol_Button,        m_style.button_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, m_style.button_hover_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  m_style.button_active_color);
    ImGui::PushStyleColor(ImGuiCol_Text,          m_style.button_text_style.color);

    const std::string& label = (m_active_tab == Tab::Login)
        ? m_style.login_button_text
        : m_style.register_button_text;

    // Append "##Action" to avoid ID collision with the tab buttons of the same name.
    std::string unique_label = label + "##Action";

    if (ImGui::Button(unique_label.c_str(), ImVec2(m_style.fields_max_width, 0))) {
        if (m_active_tab == Tab::Login) {
            executeLogin();
        } else {
            executeRegister();
        }
    }

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar();
}

// Validation

bool LoginWindow::validateLogin()
{
    std::string username(m_username);
    // Remove whitespace for validation
    username.erase(std::remove_if(username.begin(), username.end(), ::isspace), username.end());
    if (username.empty()) {
        m_error_message = m_style.error_username_empty_text;
        return false;
    }
    if (!isUsernameValid(username)) {
        m_error_message = m_style.error_username_invalid_text;
        return false;
    }
    return true;
}

bool LoginWindow::validateRegister()
{
    if (!validateLogin())   // username checks
        return false;

    std::string display_name(m_display_name);
    display_name.erase(std::remove_if(display_name.begin(), display_name.end(), ::isspace), display_name.end());
    if (display_name.empty()) {
        m_error_message = m_style.error_display_name_empty_text;
        return false;
    }

    if (!isBirthdayValid(m_birthday_year, m_birthday_month, m_birthday_day)) {
        m_error_message = m_style.error_birthday_invalid_text;
        return false;
    }
    return true;
}

bool LoginWindow::isUsernameValid(const std::string& username)
{
    for (char c : username) {
        if (!isalnum(static_cast<unsigned char>(c)))
            return false;
    }
    return true;
}

bool LoginWindow::isBirthdayValid(int year, int month, int day)
{
    if (month < 1 || month > 12) return false;
    if (day < 1) return false;

    static const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int max_day = days_in_month[month - 1];
    // Leap year adjustment for February
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) {
        max_day = 29;
    }
    if (day > max_day) return false;

    time_t now = time(nullptr);
    struct tm* now_tm = localtime(&now);
    int current_year = now_tm->tm_year + 1900;
    if (year < 1900 || year > current_year) return false;

    return true;
}

// Execution

void LoginWindow::executeLogin()
{
    m_error_message.clear();
    if (!validateLogin()) return;

    m_is_waiting = true;
    m_login_uc.execute(m_username,
        [this](bool success, const std::string& user_id_or_error, const std::string& display_name) {
            m_is_waiting = false;
            if (success)
                m_on_login(user_id_or_error, m_username, display_name);
            else
                m_error_message = user_id_or_error;
        });
}

void LoginWindow::executeRegister()
{
    m_error_message.clear();
    if (!validateRegister()) return;

    // Format birthday as YYYY-MM-DD
    char birthday_buf[16];
    snprintf(birthday_buf, sizeof(birthday_buf), "%04d-%02d-%02d",
        m_birthday_year, m_birthday_month, m_birthday_day);

    m_is_waiting = true;
    m_register_uc.execute(m_username, m_display_name, birthday_buf,
        [this](bool success, const std::string& user_id_or_error, const std::string& display_name) {
            m_is_waiting = false;
            if (success)
                m_on_login(user_id_or_error, m_username, display_name);
            else
                m_error_message = user_id_or_error;
        });
}