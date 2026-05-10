#pragma once

#include <chrono>
#include <string>

struct User {
    std::string id;
    std::string username;
    std::string roomCode;
    std::chrono::steady_clock::time_point lastMessageAt;

    User();
    User(std::string id, std::string username, std::string roomCode);
};
