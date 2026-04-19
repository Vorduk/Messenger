#include "MessengerWindow.h"

#include "imgui.h"

#include <cstring>

MessengerWindow::MessengerWindow(int width, int height, const std::string& title)
    : m_window(width, height, title)
    , m_imgui_layer(m_window)
    , m_running(false)
{
    memset(m_inputBuffer, 0, sizeof(m_inputBuffer));
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

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags dockspace_flags =
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpace", nullptr, dockspace_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id);


    ImGui::Begin("Messenger");

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

    ImGui::End(); // Messenger

    ImGui::End(); // DockSpace
}