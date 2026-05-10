AnonChat
AnonChat is a production-ready anonymous realtime web chat application built with a static vanilla frontend and a modern C++17 WebSocket backend. It supports ephemeral multi-user rooms without accounts, authentication, cookies, or a database.

Rooms and messages live in server memory only. When the final user leaves a room, the room is deleted. When the backend restarts, all chat state disappears.

Features
Anonymous chat with username and room code
Realtime messaging over WebSockets
Multiple independent rooms
Multiple users per room
Online users list
Typing indicator
Leave room flow
Responsive dark theme UI
Random room code generation
Thread-safe in-memory room handling
Message length limit
Anti-spam cooldown
Max room size enforcement
Input validation
JSON WebSocket protocol
Dockerized backend for Render
Static frontend for GitHub Pages
Tech Stack
Frontend:

HTML
CSS
Vanilla JavaScript
Backend:

C++17
Crow C++ framework
WebSockets
CMake
Docker
Deployment:

GitHub Pages for the frontend
Render Docker Web Service for the backend
Project Structure
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
How It Works
AnonChat has two separate deployable parts:

The frontend is a static web app hosted on GitHub Pages.
The backend is a C++ WebSocket server hosted on Render using Docker.
Users enter a username and room code. The browser opens a WebSocket connection to the backend:

wss://YOUR_RENDER_SERVICE.onrender.com/ws?room=ROOMCODE&username=USERNAME
The backend validates the connection, adds the user to the requested room, and broadcasts room events only to users in that room.

No messages are persisted. No database is used.

Local Development
Prerequisites
Install:

CMake
A C++17 compiler
Git
Python 3, for serving the frontend locally
Docker, optional but recommended
On macOS:

brew install cmake
On Ubuntu:

sudo apt-get update
sudo apt-get install -y build-essential cmake git
Build The Backend Locally
From the backend directory:

cd backend
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
Run the backend:

PORT=18080 CORS_ORIGIN=http://localhost:8000 ./build/anon_chat
Health check:

curl http://localhost:18080/api/health
Expected response:

{"status":"ok"}
Create a room:

curl -X POST http://localhost:18080/api/rooms
Example response:

{"roomCode":"ABCD12"}
Run The Frontend Locally
Open a second terminal:

cd frontend
python3 -m http.server 8000
Open:

http://localhost:8000
In the app, expand Connection and use:

http://localhost:18080
Docker
Build the backend image:

cd backend
docker build -t anonchat-backend .
Run the container:

docker run --rm -p 18080:18080 -e PORT=18080 -e CORS_ORIGIN=http://localhost:8000 anonchat-backend
Then test:

curl http://localhost:18080/api/health
Deployment
Backend: Render
Push this repository to GitHub.
Create a new Render Web Service.
Connect the GitHub repository.
Select Docker as the runtime.
Set the root directory to:
backend
Add environment variables:
PORT=18080
CORS_ORIGIN=https://YOUR_GITHUB_USERNAME.github.io
Deploy.
Copy the Render service URL:
https://YOUR_RENDER_SERVICE.onrender.com
Render terminates TLS automatically, so the frontend will connect with wss:// for WebSockets.

Frontend: GitHub Pages
Go to repository Settings.
Open Pages.
Select deployment from a branch.
Publish the frontend folder if available.
If GitHub Pages does not offer frontend as a source folder, move the frontend files to docs/ or use a GitHub Actions Pages workflow.
Open the deployed GitHub Pages URL.
Expand Connection.
Enter your Render backend URL:
https://YOUR_RENDER_SERVICE.onrender.com
WebSocket Protocol
Client to server:

{"type":"message","text":"Hello"}
{"type":"typing","active":true}
Server to client:

{"type":"message","username":"Ada","text":"Hello","timestamp":"2026-05-10T12:00:00Z"}
{"type":"users","users":["Ada","Grace"]}
{"type":"typing","username":"Grace","active":true}
{"type":"system","text":"Ada joined."}
{"type":"error","error":"You are sending messages too quickly."}
Backend Constraints
Username length: 2-24 characters
Room code length: 4-12 characters
Message length: 500 characters
Raw WebSocket payload limit: 2048 bytes
Anti-spam cooldown: 650 ms
Max room size: 50 users
Storage: RAM only
Persistence: none
Security And Privacy
AnonChat is intentionally anonymous and ephemeral. It does not provide user accounts, private identity, moderation tools, encryption beyond HTTPS/WSS transport, or persistent message history.

For production use:

Deploy the backend behind HTTPS.
Set CORS_ORIGIN to the exact GitHub Pages origin.
Keep message and room limits enabled.
Do not treat room codes as strong secrets.
Do not use this app for sensitive or regulated communication.
Troubleshooting
If CMake cannot find asio.hpp, remove the old build directory and rebuild:

cd backend
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
If the frontend cannot connect:

Confirm the backend is running.
Confirm the backend URL in the frontend Connection field is correct.
For local development, use http://localhost:18080.
For GitHub Pages, use the Render https:// URL.
Confirm CORS_ORIGIN matches the frontend origin.
If Docker asks for quote> in the terminal, cancel with Ctrl+C and run the command as one line:

docker run --rm -p 18080:18080 -e PORT=18080 -e CORS_ORIGIN=http://localhost:8000 anonchat-backend
License
This project is provided as an educational full-stack realtime systems project. Add a license file before publishing or distributing it for public reuse.
