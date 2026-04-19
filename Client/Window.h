#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <stdexcept>

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
    GLFWwindow* m_window = nullptr;
    int m_width, m_height;
    std::string m_title;

    void initGLFW();
    void createWindow();
    void initGLEW();

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};