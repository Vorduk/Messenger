#pragma once
#include "IDockableWindow.h"
#include "ISendMessageHandler.h"
#include "GetMessagesUseCase.h"
#include "Message.h"
#include <string>
#include <vector>
#include <mutex>

class ChatWindow : public IDockableWindow {
public:
    ChatWindow();
    const char* getName() const override { return "Chat"; }
    void Render() override;

    void SetHandler(ISendMessageHandler* handler);
    void SetUsers(const std::string& sender_id, const std::string& receiver_id, const std::string& receiver_name);
    void AddMessage(const std::string& sender, const std::string& text, bool is_local = false);
    void SetHistory(const std::vector<Message>& messages);
    void AddConfirmation(const std::string& text);
    void AddError(const std::string& text);
    void SetMessageLoader(GetMessagesUseCase* loader);

private:
    ISendMessageHandler* m_handler = nullptr;
    GetMessagesUseCase* m_message_loader = nullptr;
    std::string m_sender_id;
    std::string m_receiver_id;
    std::string m_receiver_name;
    char m_input_buffer[1024] = "";
    struct DisplayMessage { std::string text; bool is_local; };
    std::vector<DisplayMessage> m_messages;
    std::vector<std::string> m_status_messages;
    std::mutex m_mutex;
    double m_last_message_refresh = 0.0;
    static constexpr double REFRESH_INTERVAL = 5.0;
};