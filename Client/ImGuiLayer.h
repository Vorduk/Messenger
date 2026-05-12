#pragma once
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Window.h"

/**
 * Wrapper for ImGui context and rendering lifecycle tied to a GLFW window.
 * Initializes ImGui with GLFW and OpenGL3 backends, manages frame begin/end,
 * and handles optional multi-viewport platform window updates.
 */
class ImGuiLayer {
public:
    ImGuiLayer(Window& window);
    ~ImGuiLayer();

    void beginFrame();
    void endFrame();

private:
    Window& m_window;
};