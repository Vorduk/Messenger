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
    m_input_thread = std::thread(&ClientConsoleUI::inputLoop, this);
    m_loop_thread = std::thread(&ClientConsoleUI::mainLoop, this);
    m_loop_thread.join();
}

void ClientConsoleUI::stop() {
    m_is_running = false;
    if (m_input_thread.joinable())
        m_input_thread.join();
    if (m_loop_thread.joinable())
        m_loop_thread.join();
}

void ClientConsoleUI::showSendMessageConfirmation(const std::string& confirmation) {
    std::cout << "[INFO]: " << confirmation << std::endl;
}

void ClientConsoleUI::showSendMessageError(const std::string& error) {
    std::cout << "[ERROR]: " << error << std::endl;
}

void ClientConsoleUI::setSendMessageCallback(std::function<void(const std::string&, const std::string&)> callback) {
    m_send_message_callback = callback;
}


void ClientConsoleUI::inputLoop() {
    std::string line;
    while (m_is_running && std::getline(std::cin, line)) {
        if (line.empty()) continue;
        if (m_send_message_callback) {
            // Здесь можно в будущем добавить команду для выхода, например "/quit"
            m_send_message_callback("Vorduk", line);   // Имя отправителя (позже будет логин)
        }
    }
}

void ClientConsoleUI::mainLoop() {
    while (m_is_running) {
        /*std::cout << "Client UI: fuck you, user!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));*/
    }
}