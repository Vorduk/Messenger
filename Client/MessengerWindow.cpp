#include "MessengerWindow.h"

MessengerWindow::MessengerWindow(int width, int height, const std::string& title)
    : m_window(width, height, title)
    , m_imgui_layer(m_window)
{
   
}

MessengerWindow::~MessengerWindow() {
    stop();
}

void MessengerWindow::run() {
    m_running = true;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    while (m_running && !m_window.shouldClose()) {
        m_window.pollEvents();

        glClear(GL_COLOR_BUFFER_BIT);

        m_imgui_layer.beginFrame();
        renderUI();
        m_imgui_layer.endFrame();

        m_window.swapBuffers();
    }
}

void MessengerWindow::stop() {
    m_running = false;
}

void MessengerWindow::showSendMessageConfirmation(const std::string& confirmation) {
    if (m_chat_window) {
        m_chat_window->AddConfirmation(confirmation);
    }
}

void MessengerWindow::showSendMessageError(const std::string& error) {
    if (m_chat_window) {
        m_chat_window->AddError(error);
    }
}

void MessengerWindow::setSendMessageHandler(ISendMessageHandler* handler) {
    m_chat_window = std::make_unique<ChatWindow>(*handler);
    m_chat_list_window = std::make_unique<ChatListWindow>();

    m_dock_manager.RegisterWindow(m_chat_window.get());
    m_dock_manager.RegisterWindow(m_chat_list_window.get());

    using Node = DockLayoutNode;
    m_dock_manager.SetInitialLayout(
        Node::MakeSplit(ImGuiDir_Left, 0.25f,
            Node::MakeWindow(m_chat_list_window->getName()),
            Node::MakeWindow(m_chat_window->getName())
        )
    );
}

void MessengerWindow::renderUI() {
    m_dock_manager.Begin();
    m_dock_manager.RenderWindows();
    m_dock_manager.End();
}
