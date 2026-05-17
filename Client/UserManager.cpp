#include "UserManager.h"
#include "LogMacros.h"
#include "UUIDGenerator.h"
#include <random>
#include <string>

bool UserManager::login(const std::string& username) {
    std::optional<User> user = m_repository.getUserByUsername(username);
    if (user.has_value()) {
        m_current_user = std::move(user);
        LOG_INFO("User logged in: {}", m_current_user->username);
        return true;
    }
    LOG_WARN("Login failed: user '{}' not found", username);
    return false;
}

bool UserManager::registerUser(const std::string& username) {
    // Check if username already taken
    std::optional<User> existing_username = m_repository.getUserByUsername(username);
    if (existing_username.has_value()) {
        LOG_WARN("Registration failed: username '{}' already exists", username);
        return false;
    }

    User new_user;
    new_user.id = generateUUID();
    new_user.username = username;
    new_user.display_name = username; // default display name - username
    new_user.bio = "";
    new_user.avatar_path = "";
    new_user.birthday = "";
    new_user.is_online = true;      // when registered/logged in, set online
    new_user.show_online = true;
    new_user.show_typing = true;

    if (m_repository.createUser(new_user)) {
        m_current_user = new_user;
        LOG_INFO("New user registered and logged in: {}", username);
        return true;
    }
    LOG_ERROR("Failed to create user in database");
    return false;
}

std::optional<User> UserManager::getCurrentUser() const {
    return m_current_user;
}

bool UserManager::updateCurrentUser(const User& updated_user) {
    if (!m_current_user.has_value()) return false;
    if (m_repository.updateUser(updated_user)) {
        m_current_user = updated_user;
        return true;
    }
    return false;
}

void UserManager::refresh() {
    if (m_current_user.has_value()) {
        std::optional<User> refreshed = m_repository.getUserById(m_current_user->id);
        if (refreshed.has_value()) {
            m_current_user = std::move(refreshed);
        }
    }
}

void UserManager::logout() {
    if (m_current_user.has_value()) {
        // Set offline status in DB
        User user = *m_current_user;
        user.is_online = false;
        m_repository.updateUser(user);
        m_current_user.reset();
        LOG_INFO("User logged out");
    }
}