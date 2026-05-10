# AnonChat

**AnonChat** is a production-ready anonymous realtime web chat application built with a static vanilla frontend and a modern C++17 WebSocket backend.

It supports ephemeral multi-user chat rooms with **no authentication**, **no database**, and **RAM-only storage**. Rooms and messages exist only while users are connected.

---

## Features

- **Anonymous chat** with username and room code
- **Realtime messaging** using WebSockets
- **Multiple rooms** supported
- **Multiple users per room**
- **Online users list**
- **Typing indicator**
- **Leave room** functionality
- **Responsive dark theme UI**
- **Random room code generation**
- **Thread-safe room handling**
- **Message length limit**
- **Anti-spam cooldown**
- **Max room size enforcement**
- **Input validation**
- **JSON WebSocket protocol**
- **Dockerized backend**
- **GitHub Pages frontend deployment**
- **Render backend deployment**

---

## Tech Stack

### Frontend

- HTML
- CSS
- Vanilla JavaScript

### Backend

- Modern C++17
- Crow C++ Framework
- WebSockets
- CMake
- Docker

### Deployment

- GitHub Pages
- Render Docker Web Service

---

## Project Structure

```text
AnonChat/
├── frontend/
│   ├── index.html
│   ├── style.css
│   └── app.js
│
├── backend/
│   ├── src/
│   │   ├── main.cpp
│   │   ├── room.cpp
│   │   ├── room.h
│   │   ├── user.cpp
│   │   └── user.h
│   │
│   ├── Dockerfile
│   ├── CMakeLists.txt
│   └── README.md
│
├── TODO.md
└── README.md

