#pragma once
#include "IServerAPI.h"
#include <functional>

class LoginUseCase {
public:
    explicit LoginUseCase(IServerAPI& api);
    void execute(const std::string& username,
        std::function<void(bool success, const std::string& userIdOrError)> callback);
private:
    IServerAPI& m_api;
};