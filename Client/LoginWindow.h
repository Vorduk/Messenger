#pragma once
#include "IDockableWindow.h"
#include "LoginUseCase.h"
#include "RegisterUseCase.h"
#include <functional>
#include <string>

class LoginWindow : public IDockableWindow {
public:
    using LoginCallback = std::function<void(const std::string& user_id)>;
    LoginWindow(LoginUseCase& login_uc, RegisterUseCase& register_uc, LoginCallback on_login);
    const char* getName() const override { return "Login"; }
    void Render() override;
private:
    LoginUseCase& m_login_uc;
    RegisterUseCase& m_register_uc;
    LoginCallback m_on_login;
    char m_username[256] = "";
    std::string m_error_message;
    bool m_is_waiting = false;
};