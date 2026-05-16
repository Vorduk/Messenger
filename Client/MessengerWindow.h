#pragma once
#include "IUI.h"
#include "Window.h"
#include "ImGuiLayer.h"
#include "DockManager.h"
#include "ChatWindow.h"
#include "ChatListWindow.h"
#include <memory>

class MessengerWindow : public IUI {
public:
    MessengerWindow(int width, int height, const std::string& title);
    ~MessengerWindow() override;

    // IUI implementation
    void run() override;
    void stop() override;

    void showSendMessageConfirmation(const std::string& confirmation) override;
    void showSendMessageError(const std::string& error) override;
    void setSendMessageHandler(ISendMessageHandler* handler);

private:
    void renderUI();

    Window m_window; // Glfw Window
    ImGuiLayer m_imgui_layer;
    std::atomic<bool> m_running{false};
    DockManager m_dock_manager;
    std::unique_ptr<ChatWindow> m_chat_window;
    std::unique_ptr<ChatListWindow> m_chat_list_window;
};
