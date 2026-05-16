#pragma once
#include "IDockableWindow.h"
#include "ISendMessageHandler.h"
#include <string>
#include <vector>
#include <mutex>

class ChatWindow : public IDockableWindow {
public:
    explicit ChatWindow(ISendMessageHandler& sendHandler);

    const char* getName() const override { return "Chat"; }
    void Render() override;

    void AddConfirmation(const std::string& text);
    void AddError(const std::string& text);

private:
    ISendMessageHandler& m_send_handler;
    char m_input_buffer[1024] = "";
    std::vector<std::string> m_status_messages;
    std::mutex m_mutex;
};