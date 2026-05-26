#include "pch.h"
#include "MessengerServer.h"
#include "Logger.h"

#include <iostream>

void initializeOpenSSL() {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
}

int main() {
    initializeOpenSSL();
    Logger::getInstance().initialize("server.log");
    Logger::getInstance().setMinimalLevel(Logger::Level::Info);

    MessengerServer server(8080, "server.db", "server.crt", "server.key");

    server.run();
    return 0;
}