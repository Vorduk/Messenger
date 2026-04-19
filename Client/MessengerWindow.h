#pragma once
#include "IUI.h"
#include "Window.h"
#include "ImGuiLayer.h"
#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

class MessengerWindow : public IUI {
public:
    MessengerWindow(int width, int height, const std::string& title);
    ~MessengerWindow() override;

    // IUI implementation
    void run() override;
    void stop() override;

    void showSendMessageConfirmation(const std::string& confirmation) override;
    void showSendMessageError(const std::string& error) override;

    void setSendMessageCallback(
        std::function<void(const std::string& sender,
            const std::string& receiver,
            const std::string& text)> callback);

private:
    void renderUI();

    Window m_window;                //< Glfw Window
    ImGuiLayer m_imgui_layer;
    std::atomic<bool> m_running;

    char m_inputBuffer[1024] = "";

    std::function<void(const std::string&, const std::string&, const std::string&)>
        m_sendCallback;

    struct UIMessage {
        enum Type { Info, Error };
        Type type;
        std::string text;
    };

    std::vector<UIMessage> m_messages;
    std::mutex m_messagesMutex;
};