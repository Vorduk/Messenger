#pragma once
#include "IMessageSender.h"
#include <string>
#include <thread>
#include <atomic>

class MessageSender : public IMessageSender {
public:
    MessageSender(const std::string& host, int port);
    ~MessageSender();
    bool send(const std::string& serialized_message) override;
    bool connect();
private:
    int m_socket;
    std::string m_host;
    int m_port;
    std::atomic<bool> m_is_connected;
};
