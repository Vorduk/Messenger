#include "ServerAPI.h"
#include "AsyncNetworkClient.h"
#include <json.hpp>

ServerAPI::ServerAPI(AsyncNetworkClient& client) : m_client(client) {}

void ServerAPI::registerUser(const std::string& username, std::function<void(bool, const std::string&)> callback) {
    nlohmann::json req;
    req["action"] = "register";
    req["username"] = username;
    m_client.sendRequest(req.dump(), [callback](const std::string& response) {
        nlohmann::json resp = nlohmann::json::parse(response);
        if (resp["status"] == "ok") {
            callback(true, resp["user_id"]);
        }
        else {
            callback(false, resp.value("message", std::string("Unknown error")));
        }
        });
}

void ServerAPI::login(const std::string& username, std::function<void(bool, const std::string&)> callback) {
    nlohmann::json req;
    req["action"] = "login";
    req["username"] = username;
    m_client.sendRequest(req.dump(), [callback](const std::string& response) {
        nlohmann::json resp = nlohmann::json::parse(response);
        if (resp["status"] == "ok") {
            callback(true, resp["user_id"]);
        }
        else {
            callback(false, resp.value("message", std::string("Unknown error")));
        }
        });
}

void ServerAPI::getUsers(const std::string& user_id, std::function<void(std::vector<User>)> callback) {
    nlohmann::json req;
    req["action"] = "get_users";
    req["user_id"] = user_id;
    m_client.sendRequest(req.dump(), [callback](const std::string& response) {
        std::vector<User> users;
        nlohmann::json resp = nlohmann::json::parse(response);
        if (resp["status"] == "ok") {
            for (const nlohmann::json& jUser : resp["users"]) {
                User u;
                u.id = jUser["id"];
                u.username = jUser["username"];
                u.display_name = jUser.value("display_name", "");
                u.is_online = jUser.value("is_online", false);
                u.last_message = jUser.value("last_message", "");
                long long ts = jUser.value("last_message_timestamp", 0LL);
                if (ts > 0)
                    u.last_message_time = std::chrono::system_clock::time_point(std::chrono::milliseconds(ts));
                else
                    u.last_message_time = std::chrono::system_clock::time_point::min();
                users.push_back(u);
            }
        }
        callback(users);
        });
}

void ServerAPI::sendMessage(const std::string& sender_id, const std::string& receiverId,
    const std::string& text,
    std::function<void(bool, const std::string&)> callback) {
    nlohmann::json req;
    req["action"] = "send_message";
    req["sender_id"] = sender_id;
    req["receiver_id"] = receiverId;
    req["text"] = text;
    m_client.sendRequest(req.dump(), [callback](const std::string& response) {
        nlohmann::json resp = nlohmann::json::parse(response);
        if (resp["status"] == "ok") {
            callback(true, resp["message_id"]);
        }
        else {
            callback(false, resp.value("message", std::string("Unknown error")));
        }
        });
}

void ServerAPI::getMessages(const std::string& user_id, const std::string& partner_id,
    int limit, int offset,
    std::function<void(std::vector<Message>)> callback) {
    nlohmann::json req;
    req["action"] = "get_messages";
    req["user_id"] = user_id;
    req["partner_id"] = partner_id;
    req["limit"] = limit;
    req["offset"] = offset;
    m_client.sendRequest(req.dump(), [callback](const std::string& response) {
        std::vector<Message> messages;
        nlohmann::json resp = nlohmann::json::parse(response);
        if (resp["status"] == "ok") {
            for (const nlohmann::json& jMsg : resp["messages"]) {
                Message m;
                m.id = jMsg["id"];
                m.sender_id = jMsg["sender_id"];
                m.receiver_id = jMsg["receiver_id"];
                m.text = jMsg["text"];
                m.timestamp = std::chrono::system_clock::time_point(
                    std::chrono::milliseconds(jMsg["timestamp"].get<long long>()));
                m.status = static_cast<MessageStatus>(jMsg["status"].get<int>());
                messages.push_back(m);
            }
        }
        callback(messages);
        });
}