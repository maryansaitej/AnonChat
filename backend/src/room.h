#pragma once

#include "user.h"

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

namespace crow {
namespace websocket {
class connection;
}
}

struct Room {
    std::string code;
    std::chrono::system_clock::time_point createdAt;
    std::unordered_map<crow::websocket::connection*, User> users;

    Room();
    explicit Room(std::string code);

    bool isFull(std::size_t maxUsers) const;
    bool empty() const;
    std::vector<std::string> usernames() const;
};
