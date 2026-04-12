#pragma once
#include "IUI.h"
#include <atomic>
#include <thread>
#include <functional>

class ClientConsoleUI : public IUI {
public:
    ClientConsoleUI();
    ~ClientConsoleUI();
    void run() override;
    void stop() override;

    virtual void showSendMessageConfirmation(const std::string& confirmation) override;
    virtual void showSendMessageError(const std::string& error) override;

    void setSendMessageCallback(std::function<void(const std::string&, const std::string&)> callback);

private:
    void mainLoop();
    void inputLoop();
    std::atomic<bool> m_is_running;
    std::thread m_loop_thread;
    std::thread m_input_thread;
    std::function<void(const std::string&, const std::string&)> m_send_message_callback;
};