#pragma once
#include "DockableWindow.h"
#include "LoginUseCase.h"
#include "RegisterUseCase.h"
#include "StyleManager.h"
#include <functional>
#include <string>

/**
 * @brief ImGui window for user authentication (login and registration).
 *
 * Supports two tabs: Login (only username) and Register (username, display name,
 * birthday with separate day/month/year fields).
 * All visual settings are supplied via LoginWindowStyle.
 */
class LoginWindow : public DockableWindow {
public:
    /// Callback receives user_id, username and display_name after successful login/register
    using LoginCallback = std::function<void(const std::string& user_id,
        const std::string& username, const std::string& display_name)>;

    /**
     * @brief Construct the login window.
     * @param current_style Style reference (owned by StyleManager).
     * @param login_uc Login use case.
     * @param register_uc Register use case.
     * @param on_login Callback invoked on successful authentication.
     */
    LoginWindow(const LoginWindowStyle& current_style,
        LoginUseCase& login_uc, RegisterUseCase& register_uc,
        LoginCallback on_login);

    const char* getName() const override { return "Login"; }
    void render() override;

private:
    // Rendering helpers
    void renderTabs();
    void renderLoginTab();
    void renderRegisterTab();

    void renderUsernameField();        ///< Draws the username input with style
    void renderDisplayNameField();     ///< Draws the display name input (register only)
    void renderBirthdayFields();       ///< Draws three day/month/year inputs
    void renderActionButton();         ///< Draws the Login/Register button

    // Validation
    bool validateLogin();              ///< Returns true if login input is valid
    bool validateRegister();           ///< Returns true if register input is valid
    static bool isUsernameValid(const std::string& username);
    static bool isBirthdayValid(int year, int month, int day);

    // Callbacks
    void executeLogin();
    void executeRegister();

    // Style and state
    const LoginWindowStyle& m_style;   ///< Visual style (owned by StyleManager).
    LoginUseCase& m_login_uc;
    RegisterUseCase& m_register_uc;
    LoginCallback m_on_login;

    // Tab state
    enum class Tab { Login, Register };
    Tab m_active_tab = Tab::Login;

    // Input buffers
    char m_username[256] = "";
    char m_display_name[256] = "";
    int m_birthday_year = 2000;
    int m_birthday_month = 1;
    int m_birthday_day = 1;

    std::string m_error_message;
    bool m_is_waiting = false;
};