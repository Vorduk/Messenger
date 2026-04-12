#include "ClientConsoleUI.h"
#include <iostream>
#include <chrono>
#include <thread>

ClientConsoleUI::ClientConsoleUI() : m_is_running(false) {}

ClientConsoleUI::~ClientConsoleUI() {
    stop();
}

void ClientConsoleUI::run() {
    m_is_running = true;
    m_loop_thread = std::thread(&ClientConsoleUI::mainLoop, this);
    m_loop_thread.join();
}

void ClientConsoleUI::stop() {
    m_is_running = false;
    if (m_loop_thread.joinable())
        m_loop_thread.join();
}

void ClientConsoleUI::mainLoop() {
    while (m_is_running) {
        std::cout << "Client UI: fuck you, user!" << std::endl;
    }
}