#include "RegisterUseCase.h"

RegisterUseCase::RegisterUseCase(IServerAPI& api) : m_api(api) {}

void RegisterUseCase::execute(const std::string& username, const std::string& display_name, const std::string& birthday, std::function<void(bool, const std::string&, const std::string&)> callback) {
    m_api.registerUser(username, display_name, birthday, callback);
}