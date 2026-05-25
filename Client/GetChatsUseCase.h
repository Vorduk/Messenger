#pragma once
#include "IServerAPI.h"
#include "ChatListItem.h"
#include <functional>
#include <vector>

/**
 * @brief Use case for searching chats of particular user on the server.
 *
 * Delegates the actual network request to IServerAPI and returns results
 * asynchronously via a callback. All calls are non-blocking - the UI
 * remains responsive while the request is processed.
 */
class GetChatsUseCase {
public:
    /**
     * @brief Constructs the use case with a server API reference.
     * @param server_api Reference to the server communication interface.
     */
    explicit GetChatsUseCase(IServerAPI& server_api);

    /**
     * @brief Retrieves the list of existing chats for the current user.
     *
     * Sends an asynchronous request to the server and returns immediately.
     * The callback is invoked later on the main thread when results arrive.
     * Does not block the UI. Falls back to locally cached data on failure.
     *
     * @param current_user_id ID of the user whose chats are requested.
     * @param callback Called when the server responds (or on failure).
     *  - First parameter: "true" on success, "false" if server unreachable.
     *  - Second parameter: vector of ChatListItem with last message preview,
     *    timestamp, delivery status, and partner user info.
     *
     * @note Results are cached locally via IMessageRepository for offline access.
     *       If the server is unreachable, cached data is returned with "success = false".
     */
    void execute(const std::string& current_user_id, std::function<void(bool, std::vector<ChatListItem>)> callback);

private:
    IServerAPI& m_server_api;   ///< Server communication interface.
};