#pragma once
#include "IServerAPI.h"
#include "User.h"
#include <functional>
#include <vector>

class GetUsersUseCase {
public:
    explicit GetUsersUseCase(IServerAPI& api);
    void execute(const std::string& current_user_id, std::function<void(std::vector<User>)> callback);
private:
    IServerAPI& m_api;
};