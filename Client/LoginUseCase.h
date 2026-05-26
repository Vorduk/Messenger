#pragma once
#include "IServerAPI.h"
#include <functional>

class LoginUseCase {
public:
    explicit LoginUseCase(IServerAPI& api);
    // callback: (success, user_id_or_error, display_name_or_empty)
    void execute(const std::string& username, std::function<void(bool success, const std::string& user_id_or_error, const std::string& display_name)> callback);
private:
    IServerAPI& m_api;
};