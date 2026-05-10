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
```

---

## Architecture

AnonChat is split into two deployable parts:

| Layer | Technology | Deployment |
|---|---|---|
| Frontend | HTML, CSS, Vanilla JS | GitHub Pages |
| Backend | C++17, Crow, WebSockets | Render Docker Web Service |
| Storage | RAM only | Cleared on restart |

The frontend is served as static files. The backend manages rooms, connected users, WebSocket messages, typing indicators, and room cleanup entirely in memory.

---

## How It Works

1. A user enters a **username** and **room code**.
2. The browser opens a WebSocket connection to the backend.
3. The backend validates the username and room code.
4. The user is added to the matching in-memory room.
5. Messages are broadcast only to users in the same room.
6. When a user disconnects, they are removed from the room.
7. When a room becomes empty, it is deleted automatically.

No accounts, sessions, cookies, or databases are used.

---

## WebSocket URL Format

Local development:

```text
ws://localhost:18080/ws?room=ROOMCODE&username=USERNAME
```

Production:

```text
wss://YOUR_RENDER_SERVICE.onrender.com/ws?room=ROOMCODE&username=USERNAME
```

---

## Local Setup

### Prerequisites

Install:

- CMake
- A C++17 compiler
- Git
- Python 3
- Docker, optional

### macOS

```bash
brew install cmake
```

### Ubuntu

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git
```

---

## Build Backend Locally

```bash
cd backend
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

---

## Run Backend Locally

```bash
PORT=18080 CORS_ORIGIN=http://localhost:8000 ./build/anon_chat
```

Health check:

```bash
curl http://localhost:18080/api/health
```

Expected response:

```json
{
  "status": "ok"
}
```

Create a room:

```bash
curl -X POST http://localhost:18080/api/rooms
```

---

## Run Frontend Locally

Open a second terminal:

```bash
cd frontend
python3 -m http.server 8000
```

Open:

```text
http://localhost:8000
```

In the app, expand **Connection** and use:

```text
http://localhost:18080
```

---

## Docker

Build the image:

```bash
cd backend
docker build -t anonchat-backend .
```

Run the container:

```bash
docker run --rm -p 18080:18080 -e PORT=18080 -e CORS_ORIGIN=http://localhost:8000 anonchat-backend
```

Test:

```bash
curl http://localhost:18080/api/health
```

---

## Deploy Backend To Render

1. Push this repository to GitHub.
2. Open Render.
3. Create a new **Web Service**.
4. Connect your GitHub repository.
5. Select **Docker** as the runtime.
6. Set the root directory to:

```text
backend
```

7. Add environment variables:

```text
PORT=18080
CORS_ORIGIN=https://YOUR_GITHUB_USERNAME.github.io
```

8. Deploy the service.
9. Copy your Render backend URL:

```text
https://YOUR_RENDER_SERVICE.onrender.com
```

Render supports WebSockets and automatically provides HTTPS. The frontend will connect using `wss://`.

---

## Deploy Frontend To GitHub Pages

1. Open your GitHub repository.
2. Go to **Settings**.
3. Open **Pages**.
4. Select **Deploy from a branch**.
5. Choose your main branch.
6. Publish the `frontend` folder if available.
7. If GitHub Pages does not offer `frontend`, move the frontend files to `docs/` or use GitHub Actions.
8. Open your GitHub Pages URL.
9. Expand **Connection**.
10. Enter your Render backend URL:

```text
https://YOUR_RENDER_SERVICE.onrender.com
```

---

## Security And Privacy

AnonChat is intentionally anonymous and ephemeral.

It does not provide:

- User accounts
- Authentication
- Persistent message history
- Database storage
- Moderation tools
- End-to-end encryption

For production use:

- Deploy the backend over HTTPS.
- Use `wss://` WebSockets in production.
- Set `CORS_ORIGIN` to the exact GitHub Pages origin.
- Keep message and room limits enabled.
- Do not treat room codes as strong secrets.
- Do not use this app for sensitive or regulated communication.

---

## Troubleshooting

### CMake Cannot Find `asio.hpp`

Remove the old build directory and rebuild:

```bash
cd backend
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### Frontend Cannot Connect

Check:

- Backend is running.
- Backend URL is correct.
- Local backend URL is `http://localhost:18080`.
- Production backend URL is your Render `https://` URL.
- `CORS_ORIGIN` matches the frontend origin.

### Docker Shows `quote>`

Cancel with `Ctrl+C`, then run the Docker command as one line:

```bash
docker run --rm -p 18080:18080 -e PORT=18080 -e CORS_ORIGIN=http://localhost:8000 anonchat-backend
```

---

## License

This project is currently unlicensed.

Add a license before publishing or distributing it for public reuse.
