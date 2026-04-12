//#include "ServerConsoleUI.h"
//#include <iostream>

//int main() {
//    ServerConsoleUI ui;
//    ui.run();
//    return 0;
//}

// Server.cpp (временный эхо-сервер с условной компиляцией)
#include <iostream>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }
#endif

    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = { 0 };

    // Создание сокета
#ifdef _WIN32
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        WSACleanup();
        return 1;
    }
#else
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        return 1;
    }
#endif

    // Установка опции SO_REUSEADDR (в Windows тоже работает)
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // Привязка
#ifdef _WIN32
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "Bind failed" << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }
#else
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return 1;
    }
#endif

    // Прослушивание
#ifdef _WIN32
    if (listen(server_fd, 3) == SOCKET_ERROR) {
        std::cerr << "Listen failed" << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }
#else
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }
#endif

    std::cout << "Server listening on port 8080..." << std::endl;

    // Принятие соединения
#ifdef _WIN32
    client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (client_fd == INVALID_SOCKET) {
        std::cerr << "Accept failed" << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }
#else
    client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (client_fd < 0) {
        perror("accept");
        close(server_fd);
        return 1;
    }
#endif

    std::cout << "Client connected" << std::endl;

    // Цикл приёма и отправки эха
    while (true) {
        memset(buffer, 0, 1024);
#ifdef _WIN32
        int bytes = recv(client_fd, buffer, 1024, 0);
        if (bytes <= 0) break;
#else
        int bytes = recv(client_fd, buffer, 1024, 0);
        if (bytes <= 0) break;
#endif
        std::cout << "Received: " << buffer << std::endl;
        // Отправка эхо обратно
#ifdef _WIN32
        send(client_fd, buffer, bytes, 0);
#else
        send(client_fd, buffer, bytes, 0);
#endif
    }

    // Закрытие соединений и очистка
#ifdef _WIN32
    closesocket(client_fd);
    closesocket(server_fd);
    WSACleanup();
#else
    close(client_fd);
    close(server_fd);
#endif

    std::cout << "Server shut down" << std::endl;
    return 0;
}