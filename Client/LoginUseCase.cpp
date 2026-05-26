#include "pch.h"
#include "LoginUseCase.h"

LoginUseCase::LoginUseCase(IServerAPI& api) : m_api(api) {}

void LoginUseCase::execute(const std::string& username,
    std::function<void(bool, const std::string&)> callback) {
    m_api.login(username, callback);
}