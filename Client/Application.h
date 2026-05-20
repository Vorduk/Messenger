#pragma once
#include "IApplication.h"
#include "ISendMessagePresenter.h"
#include "Window.h"
#include "ImGuiLayer.h"
#include "DockableWindowsManager.h"
#include "LoginWindow.h"
#include "ChatWindow.h"
#include "ChatListWindow.h"
#include "AsyncNetworkClient.h"
#include "ServerAPI.h"
#include "SQLiteMessageRepository.h"
#include "UserManager.h"

#include "LoginUseCase.h"
#include "RegisterUseCase.h"
#include "GetUsersUseCase.h"
#include "SendMessageUseCase.h"
#include "GetMessagesUseCase.h"

#include "ISendMessageHandler.h"
#include <memory>
#include <atomic>

/**
 * @brief Main messenger application.
 */
class Application : public IApplication, public ISendMessagePresenter {
public:
    /**
     * @brief Construct the application.
     * @param width  Initial client area width.
     * @param height Initial client area height.
     * @param title  Window title string.
     */
    Application(int window_width, int window_height, const std::string& window_title);
    ~Application() override;

    void run() override;    ///< Run main loop.
    void stop() override;   ///< Stop main loop.

    void onMessageSent(const std::string& message_id, const std::string& confirmation) override; ///< Presenter callback for success.
    void onError(const std::string& error) override;                                              ///< Presenter callback for errors.

private:
    void renderUI();                                        ///< Render all registered windows.

    void onUserLoggedIn(const std::string& user_id);        ///< Called after successful login/register.
    void selectContact(const User& user);                   ///< Handle contact selection from ChatListWindow.

    std::atomic<bool> m_running{ false };                   ///< Main loop flag.

    // Windowing and rendering
    Window m_window;                                        ///< GLFW window wrapper.
    ImGuiLayer m_imgui_layer;                               ///< ImGui backend layer.

    // Network
    AsyncNetworkClient m_network;                           ///< Asynchronous TCP client.
    ServerAPI m_server_api;                                 ///< JSON-based server communication.

    // Local storage and user session
    SQLiteMessageRepository m_local_repo;                   ///< Local SQLite message cache.
    UserManager m_user_manager;                             ///< Current user session management.

    // Use cases
    std::unique_ptr<LoginUseCase> m_login_uc;               ///< Login use case.
    std::unique_ptr<RegisterUseCase> m_register_uc;         ///< Registration use case.
    std::unique_ptr<GetUsersUseCase> m_get_users_uc;        ///< Retrieve user list use case.
    std::unique_ptr<SendMessageUseCase> m_send_uc;          ///< Send message use case.
    std::unique_ptr<GetMessagesUseCase> m_get_messages_uc;  ///< Get messages use case.

    // UI components
    DockableWindowsManager m_dock_manager;                  ///< Docking layout manager.
    std::unique_ptr<LoginWindow> m_login_window;            ///< Login/register window.
    std::unique_ptr<ChatWindow> m_chat_window;              ///< Chat area window.
    std::unique_ptr<ChatListWindow> m_chat_list_window;     ///< Contact list window.

    bool m_logged_in = false;                               ///< Whether a user is currently logged in.
    std::string m_current_user_id;                          ///< ID of the logged-in user.

    std::shared_ptr<ISendMessageHandler> m_send_handler;
};