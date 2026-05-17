#pragma once
#include "IDockableWindow.h"
#include "UserManager.h"
#include <string>
#include <functional>

class LoginWindow : public IDockableWindow {
public:
    // Callback when user successfully logs in / registers
    using LoginCallback = std::function<void()>;

    LoginWindow(UserManager& user_manager, LoginCallback on_login);

    const char* getName() const override { return "Login"; }
    void Render() override;

private:
    UserManager& m_user_manager;
    LoginCallback m_on_login;
    char m_username_buffer[256] = "";
    bool m_is_login_failed = false;
};