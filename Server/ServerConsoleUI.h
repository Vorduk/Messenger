#pragma once
#include "IUI.h"
#include <atomic>
#include <thread>

class ServerConsoleUI : public IUI {
public:
    ServerConsoleUI();
    ~ServerConsoleUI();
    void run() override;
    void stop() override;
private:
    void mainLoop();
    std::atomic<bool> m_is_running;
    std::thread m_loop_thread;
};