#include "ServerConsoleUI.h"
#include <iostream>
#include <chrono>
#include <thread>

ServerConsoleUI::ServerConsoleUI() : m_is_running(false) {}

ServerConsoleUI::~ServerConsoleUI() {
    stop();
}

void ServerConsoleUI::run() {
    m_is_running = true;
    m_loop_thread = std::thread(&ServerConsoleUI::mainLoop, this);
    m_loop_thread.join();
}

void ServerConsoleUI::stop() {
    m_is_running = false;
    if (m_loop_thread.joinable())
        m_loop_thread.join();
}

void ServerConsoleUI::mainLoop() {
    while (m_is_running) {
        std::cout << "Server UI: fuck you, user!" << std::endl;
    }
}