#include "Window.h"
#include "LogMacros.h"
#include <sstream>

Window::Window(int width, int height, const std::string& title)
    : m_width(width), m_height(height), m_title(title) {
    LOG_INFO("Creating window: {} ({}x{})", title, width, height);
    initGLFW();
    createWindow();
    initGLEW();
    LOG_INFO("Window created successfully");
}

Window::~Window() {
    if (m_window) {
        LOG_INFO("Destroying window and terminating GLFW");
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }
}

void Window::initGLFW() {
    if (!glfwInit()) {
        LOG_ERROR("Failed to initialize GLFW");
        throw std::runtime_error("Failed to initialize GLFW");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void Window::createWindow() {
    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        LOG_ERROR("Failed to create GLFW window");
        throw std::runtime_error("[Error]: Failed to create GLFW window");
    }
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
}

void Window::initGLEW() {
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        throw std::runtime_error("[Error]: Failed to initialize GLEW");
    }
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::stringstream ss;
        ss << "Failed to initialize GLEW: " << glewGetErrorString(err);
        LOG_ERROR(ss.str());
        throw std::runtime_error(ss.str());
    }
    glViewport(0, 0, m_width, m_height);
    LOG_INFO("GLEW initialized successfully (version: {})",
        reinterpret_cast<const char*>(glewGetString(GLEW_VERSION)));
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::swapBuffers() {
    glfwSwapBuffers(m_window);
}

void Window::framebufferSizeCallback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}