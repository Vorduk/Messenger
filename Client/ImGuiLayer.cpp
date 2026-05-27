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

    SetupFonts();

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

void ImGuiLayer::SetupFonts() {
    ImGuiIO& io = ImGui::GetIO();

    // Очищаем стандартные шрифты (опционально)
    io.Fonts->Clear();

    // Загружаем шрифт с поддержкой русского языка
    // Первый способ: используем arialmt.ttf с указанием диапазона кириллицы
    ImFontConfig config;
    config.OversampleH = 2;
    config.OversampleV = 2;
    config.PixelSnapH = true;

    // Добавляем кириллицу в диапазон символов
    static const ImWchar ranges[] = {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0400, 0x04FF, // Cyrillic
        0,
    };

    // Загружаем ваш шрифт
    io.Fonts->AddFontFromFileTTF("fonts/ChocolateClassicalSans-Regular.ttf", 16.0f, &config, ranges);

    // Альтернативный способ: загрузить шрифт по умолчанию и добавить кириллицу
    // io.Fonts->AddFontDefault();
    // ImFontConfig config;
    // config.MergeMode = true;
    // io.Fonts->AddFontFromFileTTF("fonts/arialmt.ttf", 16.0f, &config, io.Fonts->GetGlyphRangesCyrillic());

    // Сборка шрифтов
    io.Fonts->Build();
}
