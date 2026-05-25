#pragma once
#include "IServerAPI.h"
#include "ChatListItem.h"
#include <functional>
#include <vector>

/**
 * @brief Use case for searching users on the server.
 *
 * Delegates the actual network request to IServerAPI and returns results 
 * asynchronously via a callback. All calls are non-blocking - the UI 
 * remains responsive while the request is processed.
 */
class GetUsersUseCase {
public:
    /**
     * @brief Constructs the use case with a server API reference.
     * @param server_api Reference to the server communication interface.
     */
    explicit GetUsersUseCase(IServerAPI& server_api);

    /**
     * @brief Searches for users by username or display name.
     *
     * Sends an asynchronous request to the server and returns immediately.
     * The callback is invoked later on the main thread when results arrive.
     * Does not block the UI.
     *
     * @param current_user_id ID of the current user (excluded from results).
     * @param query Search string. Examples:
     *  - "alice" — finds users with "alice" in username or display name
     *  - "john" — matches "john", "johnny", "john.doe"
     *  - "" (empty) — clears previous results, no request is sent
     *  - partial names like "ann" — matches "Ann", "Anna", "Joanna"
     * @param callback Called with "true" and matching users on success,
     *  or "false" and an empty vector if the server is unreachable.
     */
    void search(const std::string& current_user_id, const std::string& query, std::function<void(bool success, std::vector<ChatListItem>)> callback);
private:
    IServerAPI& m_server_api;  ///< Server communication interface.
};