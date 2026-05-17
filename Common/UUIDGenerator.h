#pragma once
#include <string>
#include <random>

/**
 * Generates a random UUID string.
 */
inline std::string generateUUID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static const char* hex = "0123456789abcdef";

    std::string id;
    id.reserve(36);
    for (int i = 0; i < 32; ++i) {
        id += hex[dis(gen)];
        if (i == 7 || i == 11 || i == 15 || i == 19) id += '-';
    }

    return id;
}