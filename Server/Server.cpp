#include "pch.h"
#include "TestServer.h"
#include "Logger.h"

#include <iostream>

#include <openssl/ssl.h>

int main() {
    Logger::getInstance().initialize("server.log");
    Logger::getInstance().setMinimalLevel(Logger::Level::Info);
    TestServer server(8080, "server.db");

    std::cout << "OpenSSL version: " << SSLeay_version(SSLEAY_VERSION) << std::endl;
    std::cout << "OpenSSL version number: " << SSLeay() << std::endl;

    server.run();
    return 0;
}