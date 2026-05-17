#pragma once
#include "IServerAPI.h"
#include <functional>

class RegisterUseCase {
public:
    explicit RegisterUseCase(IServerAPI& api);
    void execute(const std::string& username,
        std::function<void(bool success, const std::string& user_id_or_error)> callback);
private:
    IServerAPI& m_api;
};