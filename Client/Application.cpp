// Application.cpp
#include "Application.h"
#include "imgui.h"
#include "UUIDGenerator.h"

Application::Application(int window_width, int window_height, const std::string& window_title)
    : m_window(window_width, window_height, window_title)
    , m_imgui_layer(m_window)
    , m_network("127.0.0.1", 8080)
    , m_server_api(m_network)
    , m_local_repo("messenger.db")
    , m_user_manager(m_local_repo)
{
    // Use cases
    m_login_uc = std::make_unique<LoginUseCase>(m_server_api);
    m_register_uc = std::make_unique<RegisterUseCase>(m_server_api);
    m_get_users_uc = std::make_unique<GetUsersUseCase>(m_server_api);
    // m_send_uc will be created after login

    // Login window
    m_login_window = std::make_unique<LoginWindow>(*m_login_uc, *m_register_uc,
        [this](const std::string& user_id) { onUserLoggedIn(user_id); });
    m_dock_manager.addWindow(m_login_window.get());
    using Node = DockLayoutNode;
    m_dock_manager.setCurrentLayout(Node::makeWindow("Login"));
}

Application::~Application() { stop(); }

void Application::run() {
    m_running = true;
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    while (m_running && !m_window.shouldClose()) {
        m_window.pollEvents();
        m_network.pollCallbacks();       // Process server responses in main thread
        glClear(GL_COLOR_BUFFER_BIT);
        m_imgui_layer.beginFrame();
        renderUI();
        m_imgui_layer.endFrame();
        m_window.swapBuffers();
    }
}

void Application::stop() { m_running = false; }

void Application::onMessageSent(const std::string& message_id, const std::string& confirmation) {
    if (m_chat_window) m_chat_window->AddConfirmation(confirmation);
}

void Application::onError(const std::string& error) {
    if (m_chat_window) m_chat_window->AddError(error);
}

void Application::onUserLoggedIn(const std::string& user_id) {
    m_current_user_id = user_id;
    m_logged_in = true;

    m_dock_manager.removeWindow("Login");
    m_login_window.reset();

    // Send use case
    m_send_uc = std::make_unique<SendMessageUseCase>(m_server_api, *this, m_local_repo);
    // Get messages use case
    m_get_messages_uc = std::make_unique<GetMessagesUseCase>(m_server_api);

    // Chat windows
    m_chat_window = std::make_unique<ChatWindow>();

    // Обработчик отправки – теперь время жизни контролируется через shared_ptr
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

    // Передаём загрузчик сообщений для автообновления
    m_chat_window->SetMessageLoader(m_get_messages_uc.get());

    m_chat_list_window = std::make_unique<ChatListWindow>(*m_get_users_uc, m_current_user_id);
    m_chat_list_window->SetOnUserSelected([this](const User& user) { selectContact(user); });

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
    if (m_chat_window) {
        m_chat_window->SetUsers(m_current_user_id, user.id, user.display_name);
        
        m_get_messages_uc->execute(m_current_user_id, user.id, 50, 0,
            [this](std::vector<Message> messages) {
                m_chat_window->SetHistory(messages);
            });
    }
}

void Application::renderUI() {
    m_dock_manager.begin();
    m_dock_manager.renderWindows();
    m_dock_manager.end();
}