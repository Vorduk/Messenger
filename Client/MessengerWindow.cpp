#include "MessengerWindow.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <cstring>

MessengerWindow::MessengerWindow(int width, int height, const std::string& title)
    : m_window(width, height, title)
    , m_imgui_layer(m_window)
    , m_running(false)
{
    memset(m_inputBuffer, 0, sizeof(m_inputBuffer));

    m_dock_manager.RegisterWindow("Chat", [this]() {

        ImGui::InputText("Message", m_inputBuffer, IM_ARRAYSIZE(m_inputBuffer));
        if (ImGui::Button("Send")) {
            if (m_sendCallback && strlen(m_inputBuffer) > 0) {
                m_sendCallback("User", "Server", m_inputBuffer);
                memset(m_inputBuffer, 0, sizeof(m_inputBuffer));
            }
        }
        ImGui::Separator();
        ImGui::Text("Status:");
        {
            std::lock_guard<std::mutex> lock(m_messagesMutex);
            for (const auto& msg : m_messages) {
                if (msg.type == UIMessage::Error) {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "[ERROR] %s", msg.text.c_str());
                }
                else {
                    ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "[INFO] %s", msg.text.c_str());
                }
            }
        }
        });
    m_dock_manager.RegisterWindow("Chat List", [this]() {

        ImGui::Text("Chat List placeholder");
        });

    using Node = DockLayoutNode;
    m_dock_manager.SetInitialLayout(Node::MakeSplit(ImGuiDir_Left, 0.25f, Node::MakeWindow("Chat List"), Node::MakeWindow("Chat")));
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
    std::lock_guard<std::mutex> lock(m_messagesMutex);
    m_messages.push_back({ UIMessage::Info, confirmation });
    if (m_messages.size() > 20) {
        m_messages.erase(m_messages.begin());
    }
}

void MessengerWindow::showSendMessageError(const std::string& error) {
    std::lock_guard<std::mutex> lock(m_messagesMutex);
    m_messages.push_back({ UIMessage::Error, error });
    if (m_messages.size() > 20) {
        m_messages.erase(m_messages.begin());
    }
}

void MessengerWindow::setSendMessageCallback(
    std::function<void(const std::string&, const std::string&, const std::string&)> callback) {
    m_sendCallback = std::move(callback);
}

void MessengerWindow::renderUI() {
    m_dock_manager.Begin();
    m_dock_manager.RenderWindows();
    m_dock_manager.End();
}