#pragma once
#include "IUI.h"
#include <atomic>
#include <thread>
#include <functional>

// Test
#include <sqlite3.h>
#include <iostream>

class ClientConsoleUI : public IUI {
public:
    ClientConsoleUI();
    ~ClientConsoleUI();
    void run() override;
    void stop() override;

    virtual void showSendMessageConfirmation(const std::string& confirmation) override;
    virtual void showSendMessageError(const std::string& error) override;

    void setSendMessageCallback(std::function<void(const std::string&, const std::string&, const std::string&)> callback);

    // Test
    void testSQLite() {
        sqlite3* db;
        int rc = sqlite3_open("test.db", &db);
        if (rc == SQLITE_OK) {
            std::cout << "SQLite version: " << sqlite3_libversion() << std::endl;
            sqlite3_close(db);
        }
        else {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        }
    }

private:
    void mainLoop();
    void inputLoop();
    std::atomic<bool> m_is_running;
    std::thread m_loop_thread;
    std::thread m_input_thread;
    std::function<void(const std::string&, const std::string&, const std::string&)> m_send_message_callback;
};