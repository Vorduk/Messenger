#pragma once
#include "IMessageRepository.h"
#include <string>
#include <optional>

class UserManager {
public:
    explicit UserManager(IMessageRepository& repository);

    // Attempt to log in by username. Returns true if user exists.
    bool login(const std::string& username);

    // Register a new user with the given username.
    // display_name is set equal to username initially.
    bool registerUser(const std::string& username);

    // Returns current logged-in user, if any.
    std::optional<User> getCurrentUser() const;

    // Update current user's fields (display name, bio, privacy settings, etc.)
    bool updateCurrentUser(const User& updated_user);

    // Refresh current user data from repository
    void refresh();

    // Logout
    void logout();

private:
    IMessageRepository& m_repository;
    std::optional<User> m_current_user;
};