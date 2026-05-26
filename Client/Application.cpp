#include "pch.h"
#include "Application.h"
#include "imgui.h"
#include "UUIDGenerator.h"
#include "LogMacros.h"
#include "StyleManager.h"

Application::Application(int window_width, int window_height, const std::string& window_title)
    : m_window(window_width, window_height, window_title)
    , m_imgui_layer(m_window)
    , m_network("127.0.0.1", 8080, true)
    , m_server_api(m_network)
    , m_local_repo("messenger.db")
    , m_user_manager(m_local_repo)
{
    m_network.enableTls();

    // Use cases
    m_login_uc = std::make_unique<LoginUseCase>(m_server_api);
    m_register_uc = std::make_unique<RegisterUseCase>(m_server_api);
    m_get_users_uc = std::make_unique<GetUsersUseCase>(m_server_api);
    // m_send_uc will be created after login

    // Login window
    m_login_window = std::make_unique<LoginWindow>(*m_login_uc, *m_register_uc,
        [this](const std::string& user_id, const std::string& username, const std::string& display_name) {
            onUserLoggedIn(user_id, username, display_name);
        });
    m_dock_manager.addWindow(m_login_window.get());
    using Node = DockLayoutNode;
    m_dock_manager.setCurrentLayout(Node::makeWindow("Login"));

    // Set up automatic re-login after reconnection
    m_network.setReconnectCallback([this]() {
        if (!m_current_username.empty()) {
            m_login_uc->execute(m_current_username,
                [this](bool success, const std::string& user_id_or_error, const std::string& display_name) {
                    if (success) {
                        m_current_user_id = user_id_or_error;
                        m_current_display_name = display_name;  // update display name too
                        LOG_INFO("Re-logged in after reconnection");
                        if (m_chat_list_window) m_chat_list_window->refreshUsers();
                    }
                    else {
                        LOG_ERROR("Re-login after reconnection failed: {}", user_id_or_error);
                    }
                });
        }
    }
    );
}

Application::~Application() { 
    stop(); 
}

void Application::run() {
    m_running = true;
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);   // Initial color.
    while (m_running && !m_window.shouldClose()) {
        m_window.pollEvents();          // Glfw window poll events.
        m_network.pollCallbacks();      // Process server responses.
        glClear(GL_COLOR_BUFFER_BIT);
        m_imgui_layer.beginFrame();     // Imgui begin frame
        renderUI();                     // Imgui windows begin, render and end.
        m_imgui_layer.endFrame();       // Imgui begin frame.
        m_window.swapBuffers();         // Swap screen buffers.
    }
}

void Application::stop() { 
    m_running = false; 
}

void Application::onMessageSent(const std::string& message_id, const std::string& message_text, const std::string& confirmation) {

    if (m_chat_list_window) {
        m_chat_list_window->updateLastMessage(m_current_chat_partner_id, message_text, std::chrono::system_clock::now(), MessageStatus::Sent);
    }
}

void Application::onError(const std::string& error) {
    return;
}

void Application::renderUI() {
    m_dock_manager.begin();
    m_dock_manager.renderWindows();
    m_dock_manager.end();
}

void Application::onUserLoggedIn(const std::string& user_id, const std::string& username, const std::string& display_name) {
    m_current_user_id = user_id;
    m_current_username = username;
    m_current_display_name = display_name;
    m_logged_in = true;

    m_dock_manager.removeWindow("Login");
    m_login_window.reset();

    m_send_uc = std::make_unique<SendMessageUseCase>(m_server_api, *this, m_local_repo);
    m_get_messages_uc = std::make_unique<GetMessagesUseCase>(m_server_api, m_local_repo);

    StyleManager& style_manager = StyleManager::getInstance();

    m_chat_window = std::make_unique<ChatWindow>(style_manager.getChatWindowStyle());

    struct SendHandler : ISendMessageHandler {
        SendMessageUseCase& m_uc;
        explicit SendHandler(SendMessageUseCase& uc) : m_uc(uc) {}
        void onUserSendMessage(const std::string& sender,
            const std::string& receiver,
            const std::string& text) override {
            m_uc.execute(sender, receiver, text);
        }
    };
    m_send_handler = std::make_shared<SendHandler>(*m_send_uc);
    m_chat_window->SetHandler(m_send_handler.get());
    m_chat_window->SetMessageLoader(m_get_messages_uc.get());

    m_get_chats_uc = std::make_unique<GetChatsUseCase>(m_server_api);

    m_chat_list_window = std::make_unique<ChatListWindow>(
        style_manager.getChatListWindowStyle(),
        *m_get_users_uc,
        *m_get_chats_uc,
        m_local_repo,
        m_current_user_id
    );
    m_chat_list_window->setOnUserSelected([this](const User& user) { selectContact(user); });

    m_dock_manager.addWindow(m_chat_window.get());
    m_dock_manager.addWindow(m_chat_list_window.get());

    using Node = DockLayoutNode;
    m_dock_manager.setCurrentLayout(
        Node::makeSplit(ImGuiDir_Left, 0.25f,
            Node::makeWindow(m_chat_list_window->getName()),
            Node::makeWindow(m_chat_window->getName())
        )
    );
}

void Application::selectContact(const User& user) {
    m_current_chat_partner_id = user.id;
    if (m_chat_window) {
        m_chat_window->SetUsers(m_current_user_id, user.id, user.display_name);
        m_get_messages_uc->execute(m_current_user_id, user.id, 50, 0,
            [this](std::vector<Message> messages) {
                m_chat_window->SetHistory(messages);
            });
    }
}

