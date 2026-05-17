#pragma once
#include "IUI.h"
#include "Window.h"
#include "ImGuiLayer.h"
#include "DockManager.h"
#include "ChatWindow.h"
#include "ChatListWindow.h"
#include "ISendMessageHandler.h"
#include <memory>
#include <atomic>

/**
 * @brief Main application window implementing the IUI interface.
 */
class MessengerWindow : public IUI {
public:
    /**
     * @brief Construct the main window.
     * @param width  Initial client area width.
     * @param height Initial client area height.
     * @param title  Window title string.
     */
    MessengerWindow(int width, int height, const std::string& title);
    ~MessengerWindow() override;

    // IUI implementation
    void run() override;
    void stop() override;

    void showSendMessageConfirmation(const std::string& confirmation) override;
    void showSendMessageError(const std::string& error) override;

    /**
     * @brief Provide the handler for sending messages and initialise child windows.
     *
     * Must be called before run(). Creates ChatWindow and ChatListWindow,
     * registers them with the docking manager, and applies the initial split layout.
     *
     * @param handler Pointer to the send message handler (usually the controller).
     */
    void setSendMessageHandler(ISendMessageHandler* handler);

private:
    /**
     * @brief Single frame rendering: begin dockspace, draw windows, end dockspace.
     */
    void renderUi();

    Window m_window;                            ///< GLFW window wrapper.
    ImGuiLayer m_imgui_layer;                   ///< ImGui backend layer.
    std::atomic<bool> m_running{ false };       ///< Main loop guard.

    DockManager m_dock_manager;                 ///< Docking layout manager.
    std::unique_ptr<ChatWindow> m_chat_window;  ///< Chat area window (created after handler is set).
    std::unique_ptr<ChatListWindow> m_chat_list_window;     ///< Contact list window (created after handler is set).
};