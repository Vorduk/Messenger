#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <stdexcept>

/**
 * @brief Wrapper for a GLFW window with an OpenGL context.
 * Creates and manages a GLFW window, initializes GLEW, and provides
 * basic frame operations (polling events, swapping buffers).
 * Non-copyable.
 */
class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool shouldClose() const;
    void pollEvents();
    void swapBuffers();
    GLFWwindow* getHandle() const { return m_window; }

private:
    // Window params
    GLFWwindow* m_window = nullptr;
    int m_width, m_height;
    std::string m_title;

    // Initializers
    void initGLFW();
    void createWindow();
    void initGLEW();

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};