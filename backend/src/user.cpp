#include "user.h"

#include <utility>

User::User()
    : lastMessageAt(std::chrono::steady_clock::now() - std::chrono::seconds(10)) {}

User::User(std::string id, std::string username, std::string roomCode)
    : id(std::move(id)),
      username(std::move(username)),
      roomCode(std::move(roomCode)),
      lastMessageAt(std::chrono::steady_clock::now() - std::chrono::seconds(10)) {}
