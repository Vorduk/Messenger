#pragma once
#include "DockableWindow.h"
#include "LoginUseCase.h"
#include "RegisterUseCase.h"
#include <functional>
#include <string>

class LoginWindow : public DockableWindow {
public:
    /// Callback receives user_id, username and display_name after successful login/register
    using LoginCallback = std::function<void(const std::string& user_id, const std::string& username, const std::string& display_name)>;
    LoginWindow(LoginUseCase& login_uc, RegisterUseCase& register_uc, LoginCallback on_login);
    const char* getName() const override;
    void render() override;
private:
    LoginUseCase& m_login_uc;
    RegisterUseCase& m_register_uc;
    LoginCallback m_on_login;
    char m_username[256] = "";
    char m_display_name[256] = "";   // Display name
    char m_birthday[64] = "";        // Birthday (YYYY-MM-DD)
    std::string m_error_message;
    bool m_is_waiting = false;
};