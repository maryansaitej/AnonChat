#include "room.h"
#include "user.h"

#include <crow.h>

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <cstddef>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <random>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

constexpr std::size_t kMaxMessageLength = 500;
constexpr std::size_t kMaxRoomSize = 50;
constexpr int kMessageCooldownMs = 650;

std::mutex gRoomsMutex;
std::unordered_map<std::string, Room> gRooms;

struct ConnectionInfo {
    std::string roomCode;
    std::string username;
};

std::string envOrDefault(const char* name, const std::string& fallback) {
    const char* value = std::getenv(name);
    if (value == nullptr || std::string(value).empty()) {
        return fallback;
    }
    return value;
}

int portFromEnv() {
    const std::string value = envOrDefault("PORT", "18080");
    try {
        return std::stoi(value);
    } catch (...) {
        return 18080;
    }
}

bool originAllowed(const crow::request& req) {
    const std::string allowedOrigin = envOrDefault("CORS_ORIGIN", "*");
    if (allowedOrigin == "*") {
        return true;
    }

    const std::string origin = req.get_header_value("Origin");
    return origin.empty() || origin == allowedOrigin;
}

void addCors(crow::response& res) {
    const std::string allowedOrigin = envOrDefault("CORS_ORIGIN", "*");
    res.add_header("Access-Control-Allow-Origin", allowedOrigin);
    res.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.add_header("Access-Control-Allow-Headers", "Content-Type");
    res.add_header("Access-Control-Max-Age", "86400");
}

std::string uppercase(std::string input) {
    std::transform(input.begin(), input.end(), input.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return input;
}

bool validRoomCode(const std::string& code) {
    static const std::regex pattern("^[A-Z0-9]{4,12}$");
    return std::regex_match(code, pattern);
}

bool validUsername(const std::string& username) {
    static const std::regex pattern("^[A-Za-z0-9 _.-]{2,24}$");
    return std::regex_match(username, pattern);
}

std::string trimToLimit(const std::string& text, std::size_t maxLength) {
    if (text.size() <= maxLength) {
        return text;
    }
    return text.substr(0, maxLength);
}

std::string generateId() {
    static thread_local std::mt19937_64 rng(std::random_device{}());
    static const char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::uniform_int_distribution<std::size_t> dist(0, sizeof(alphabet) - 2);

    std::string id;
    id.reserve(18);
    for (int i = 0; i < 18; ++i) {
        id.push_back(alphabet[dist(rng)]);
    }
    return id;
}

std::string generateRoomCode() {
    static thread_local std::mt19937 rng(std::random_device{}());
    static const char alphabet[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    std::uniform_int_distribution<std::size_t> dist(0, sizeof(alphabet) - 2);

    for (int attempt = 0; attempt < 100; ++attempt) {
        std::string code;
        code.reserve(6);
        for (int i = 0; i < 6; ++i) {
            code.push_back(alphabet[dist(rng)]);
        }

        std::lock_guard<std::mutex> lock(gRoomsMutex);
        if (gRooms.find(code) == gRooms.end()) {
            return code;
        }
    }

    throw std::runtime_error("Unable to generate room code");
}

std::string jsonString(const std::string& value) {
    std::ostringstream out;
    out << '"';
    for (const char c : value) {
        switch (c) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    out << "\\u00";
                    const char* hex = "0123456789abcdef";
                    const auto byte = static_cast<unsigned char>(c);
                    out << hex[(byte >> 4) & 0x0f] << hex[byte & 0x0f];
                } else {
                    out << c;
                }
        }
    }
    out << '"';
    return out.str();
}

std::string nowIso8601() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &time);
#else
    gmtime_r(&time, &tm);
#endif
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &tm);
    return buffer;
}

std::string usersPayload(const std::vector<std::string>& usernames) {
    std::ostringstream out;
    out << R"({"type":"users","users":[)";
    for (std::size_t i = 0; i < usernames.size(); ++i) {
        if (i > 0) {
            out << ',';
        }
        out << jsonString(usernames[i]);
    }
    out << "]}";
    return out.str();
}

std::string systemPayload(const std::string& text) {
    return std::string(R"({"type":"system","text":)") + jsonString(text) + "}";
}

std::string errorPayload(const std::string& text) {
    return std::string(R"({"type":"error","error":)") + jsonString(text) + "}";
}

std::vector<crow::websocket::connection*> roomConnectionsUnlocked(const std::string& roomCode) {
    std::vector<crow::websocket::connection*> connections;
    auto roomIt = gRooms.find(roomCode);
    if (roomIt == gRooms.end()) {
        return connections;
    }

    connections.reserve(roomIt->second.users.size());
    for (auto& entry : roomIt->second.users) {
        connections.push_back(entry.first);
    }
    return connections;
}

void sendToConnections(const std::vector<crow::websocket::connection*>& connections, const std::string& payload) {
    for (auto* conn : connections) {
        if (conn != nullptr) {
            conn->send_text(payload);
        }
    }
}

void broadcastToRoom(const std::string& roomCode, const std::string& payload) {
    std::vector<crow::websocket::connection*> connections;
    {
        std::lock_guard<std::mutex> lock(gRoomsMutex);
        connections = roomConnectionsUnlocked(roomCode);
    }
    sendToConnections(connections, payload);
}

void broadcastUsers(const std::string& roomCode) {
    std::vector<crow::websocket::connection*> connections;
    std::vector<std::string> usernames;
    {
        std::lock_guard<std::mutex> lock(gRoomsMutex);
        auto roomIt = gRooms.find(roomCode);
        if (roomIt == gRooms.end()) {
            return;
        }
        connections = roomConnectionsUnlocked(roomCode);
        usernames = roomIt->second.usernames();
    }
    sendToConnections(connections, usersPayload(usernames));
}

std::string queryParam(const crow::request& req, const char* name) {
    const char* value = req.url_params.get(name);
    return value == nullptr ? "" : std::string(value);
}

}  // namespace

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/api/health").methods(crow::HTTPMethod::GET, crow::HTTPMethod::OPTIONS)
    ([](const crow::request& req) {
        crow::response res;
        addCors(res);
        if (req.method == crow::HTTPMethod::OPTIONS) {
            res.code = 204;
            return res;
        }
        res.code = 200;
        res.set_header("Content-Type", "application/json");
        res.body = R"({"status":"ok"})";
        return res;
    });

    CROW_ROUTE(app, "/api/rooms").methods(crow::HTTPMethod::POST, crow::HTTPMethod::OPTIONS)
    ([](const crow::request& req) {
        crow::response res;
        addCors(res);
        if (req.method == crow::HTTPMethod::OPTIONS) {
            res.code = 204;
            return res;
        }
        if (!originAllowed(req)) {
            res.code = 403;
            res.body = R"({"error":"Origin not allowed"})";
            return res;
        }

        try {
            const std::string code = generateRoomCode();
            res.code = 201;
            res.set_header("Content-Type", "application/json");
            res.body = std::string(R"({"roomCode":)") + jsonString(code) + "}";
        } catch (const std::exception& ex) {
            res.code = 500;
            res.set_header("Content-Type", "application/json");
            res.body = errorPayload(ex.what());
        }
        return res;
    });

    CROW_ROUTE(app, "/ws")
    .websocket(&app)
    .onaccept([](const crow::request& req, void** userdata) {
        if (!originAllowed(req)) {
            return false;
        }

        const std::string roomCode = uppercase(queryParam(req, "room"));
        const std::string username = queryParam(req, "username");
        if (!validRoomCode(roomCode) || !validUsername(username)) {
            return false;
        }

        std::lock_guard<std::mutex> lock(gRoomsMutex);
        auto roomIt = gRooms.find(roomCode);
        if (roomIt != gRooms.end() && roomIt->second.isFull(kMaxRoomSize)) {
            return false;
        }

        *userdata = new ConnectionInfo{roomCode, username};
        return true;
    })
    .onopen([](crow::websocket::connection& conn) {
        auto* info = static_cast<ConnectionInfo*>(conn.userdata());
        if (info == nullptr) {
            conn.close("Missing connection state");
            return;
        }

        const std::string roomCode = info->roomCode;
        const std::string username = info->username;
        const User user(generateId(), username, roomCode);
        {
            std::lock_guard<std::mutex> lock(gRoomsMutex);
            auto [roomIt, inserted] = gRooms.try_emplace(roomCode, Room(roomCode));
            (void)inserted;
            roomIt->second.users[&conn] = user;
        }

        broadcastToRoom(roomCode, systemPayload(username + " joined."));
        broadcastUsers(roomCode);
    })
    .onmessage([](crow::websocket::connection& conn, const std::string& data, bool isBinary) {
        if (isBinary || data.size() > 2048) {
            conn.send_text(errorPayload("Invalid message."));
            return;
        }

        const auto body = crow::json::load(data);
        if (!body) {
            conn.send_text(errorPayload("Invalid JSON."));
            return;
        }

        std::string roomCode;
        std::string username;
        {
            std::lock_guard<std::mutex> lock(gRoomsMutex);
            for (auto& roomEntry : gRooms) {
                auto userIt = roomEntry.second.users.find(&conn);
                if (userIt != roomEntry.second.users.end()) {
                    roomCode = roomEntry.first;
                    username = userIt->second.username;
                    break;
                }
            }
        }

        if (roomCode.empty()) {
            conn.send_text(errorPayload("You are not in a room."));
            return;
        }

        const std::string type = body.has("type") && body["type"].t() == crow::json::type::String
            ? std::string(body["type"].s())
            : "";

        if (type == "typing") {
            const bool active = body.has("active") && body["active"].b();
            broadcastToRoom(roomCode, std::string(R"({"type":"typing","username":)") +
                jsonString(username) + R"(,"active":)" + (active ? "true" : "false") + "}");
            return;
        }

        if (type != "message" || !body.has("text") || body["text"].t() != crow::json::type::String) {
            conn.send_text(errorPayload("Unsupported payload."));
            return;
        }

        const auto now = std::chrono::steady_clock::now();
        {
            std::lock_guard<std::mutex> lock(gRoomsMutex);
            auto roomIt = gRooms.find(roomCode);
            if (roomIt == gRooms.end()) {
                conn.send_text(errorPayload("Room no longer exists."));
                return;
            }
            auto userIt = roomIt->second.users.find(&conn);
            if (userIt == roomIt->second.users.end()) {
                conn.send_text(errorPayload("User no longer exists."));
                return;
            }
            const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - userIt->second.lastMessageAt).count();
            if (elapsed < kMessageCooldownMs) {
                conn.send_text(errorPayload("You are sending messages too quickly."));
                return;
            }
            userIt->second.lastMessageAt = now;
        }

        std::string text = trimToLimit(body["text"].s(), kMaxMessageLength);
        text.erase(text.begin(), std::find_if(text.begin(), text.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        text.erase(std::find_if(text.rbegin(), text.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), text.end());

        if (text.empty()) {
            conn.send_text(errorPayload("Message cannot be empty."));
            return;
        }

        const std::string payload = std::string(R"({"type":"message","username":)") +
            jsonString(username) + R"(,"text":)" + jsonString(text) +
            R"(,"timestamp":)" + jsonString(nowIso8601()) + "}";
        broadcastToRoom(roomCode, payload);
    })
    .onclose([](crow::websocket::connection& conn, const std::string&) {
        std::string roomCode;
        std::string username;
        bool deletedRoom = false;
        {
            std::lock_guard<std::mutex> lock(gRoomsMutex);
            for (auto roomIt = gRooms.begin(); roomIt != gRooms.end(); ++roomIt) {
                auto userIt = roomIt->second.users.find(&conn);
                if (userIt == roomIt->second.users.end()) {
                    continue;
                }
                roomCode = roomIt->first;
                username = userIt->second.username;
                roomIt->second.users.erase(userIt);
                if (roomIt->second.empty()) {
                    gRooms.erase(roomIt);
                    deletedRoom = true;
                }
                break;
            }
        }

        if (!roomCode.empty() && !deletedRoom) {
            broadcastToRoom(roomCode, systemPayload(username + " left."));
            broadcastUsers(roomCode);
        }

        delete static_cast<ConnectionInfo*>(conn.userdata());
        conn.userdata(nullptr);
    });

    const int port = portFromEnv();
    std::cout << "Anonymous chat backend listening on port " << port << std::endl;
    app.port(static_cast<uint16_t>(port)).multithreaded().run();
}
