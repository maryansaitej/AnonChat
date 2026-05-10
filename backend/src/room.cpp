#include "room.h"

#include <algorithm>
#include <utility>

Room::Room() : createdAt(std::chrono::system_clock::now()) {}

Room::Room(std::string code)
    : code(std::move(code)), createdAt(std::chrono::system_clock::now()) {}

bool Room::isFull(std::size_t maxUsers) const {
    return users.size() >= maxUsers;
}

bool Room::empty() const {
    return users.empty();
}

std::vector<std::string> Room::usernames() const {
    std::vector<std::string> names;
    names.reserve(users.size());
    for (const auto& entry : users) {
        names.push_back(entry.second.username);
    }
    std::sort(names.begin(), names.end());
    return names;
}
