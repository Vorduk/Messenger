#include "MessengerWindow.h"
#include "imgui.h"

MessengerWindow::MessengerWindow(int width, int height, const std::string& title)
    : m_window(width, height, title)
    , m_imgui_layer(m_window)
{
    // Child windows are created later.
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
        renderUi();
        m_imgui_layer.endFrame();

        m_window.swapBuffers();
    }
}

void MessengerWindow::stop() {
    m_running = false;
}

void MessengerWindow::showSendMessageConfirmation(const std::string& confirmation) {
    // Delegate to the chat window if it exists.
    if (m_chat_window) {
        m_chat_window->AddConfirmation(confirmation);
    }
}

void MessengerWindow::showSendMessageError(const std::string& error) {
    // Delegate to the chat window if it exists.
    if (m_chat_window) {
        m_chat_window->AddError(error);
    }
}

void MessengerWindow::setSendMessageHandler(ISendMessageHandler* handler) {
    // Create the two main child windows.
    m_chat_window = std::make_unique<ChatWindow>(*handler);
    m_chat_list_window = std::make_unique<ChatListWindow>();

    // Register them with the docking manager.
    m_dock_manager.addWindow(m_chat_window.get());
    m_dock_manager.addWindow(m_chat_list_window.get());

    // Define and apply the initial layout: Chat List on the left (25%), Chat on the right.
    using Node = DockLayoutNode;
    m_dock_manager.setCurrentLayout(
        Node::makeSplit(ImGuiDir_Left, 0.25f,
            Node::makeWindow(m_chat_list_window->getName()),
            Node::makeWindow(m_chat_window->getName())
        )
    );
}

void MessengerWindow::renderUi() {
    m_dock_manager.begin();
    m_dock_manager.renderWindows();
    m_dock_manager.end();
}