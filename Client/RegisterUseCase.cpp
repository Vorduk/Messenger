#include "RegisterUseCase.h"

RegisterUseCase::RegisterUseCase(IServerAPI& api) : m_api(api) {}

void RegisterUseCase::execute(const std::string& username, std::function<void(bool, const std::string&)> callback) {
    m_api.registerUser(username, callback);
}