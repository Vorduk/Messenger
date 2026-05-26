#include "pch.h"
#include "ImGuiLayer.h"

ImGuiLayer::ImGuiLayer(Window& window) : m_window(window) {
    IMGUI_CHECKVERSION();       // Check if imgui headers and library match.
    ImGui::CreateContext();     // Create ImGui context (one for thread).

    ImGuiIO& io = ImGui::GetIO();                           // Get io structure.
    //io.IniFilename = nullptr;                             // Do not save window parameters for restart
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable keyboard navigation.
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // Enable docking.
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Enable multiple windows.
    
    ImGui::StyleColorsDark();   // Dark theme for default.

    ImGui_ImplGlfw_InitForOpenGL(m_window.getHandle(), true);   // Synchronize with Glfw.
    ImGui_ImplOpenGL3_Init("#version 330");                     // OpenGL version.
}

ImGuiLayer::~ImGuiLayer() {
    // Shutdown everything and destroy context.
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::beginFrame() {
    // New frame for opengl, glfw, imgui.
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update secondary windows. Essential for working viewport.
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}