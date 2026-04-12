#pragma once
#include "IUI.h"
#include <atomic>
#include <thread>

class ClientConsoleUI : public IUI {
public:
    ClientConsoleUI();
    ~ClientConsoleUI();
    void run() override;
    void stop() override;
private:
    void mainLoop();
    std::atomic<bool> m_is_running;
    std::thread m_loop_thread;
};