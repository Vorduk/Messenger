#include "TestServer.h"
#include "Logger.h"

int main() {
    Logger::getInstance().initialize("server.log");
    Logger::getInstance().setMinimalLevel(Logger::Level::Info);
    TestServer server(8080, "server.db");
    server.run();
    return 0;
}