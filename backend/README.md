# Anonymous Realtime Chat Backend

Modern C++17 backend for an anonymous, RAM-only, multi-room WebSocket chat application. It uses Crow, CMake, Docker, and Render's Docker deployment flow.

## Architecture

- `POST /api/rooms` generates a random room code.
- `GET /api/health` returns backend health.
- `GET /ws?room=ROOM&username=NAME` upgrades to a WebSocket connection.
- `std::unordered_map<std::string, Room>` stores all rooms.
- Each `Room` stores connected users by `crow::websocket::connection*`.
- A global `std::mutex` protects all room and user mutations.
- Rooms are deleted automatically when the last user disconnects.
- A generated room code does not create persistent state until the first WebSocket user joins it.
- No database, sessions, cookies, or authentication are used.

## JSON WebSocket Protocol

Client to server:

```json
{"type":"message","text":"Hello"}
{"type":"typing","active":true}
{"type":"typing","active":false}
```

Server to client:

```json
{"type":"message","username":"Ada","text":"Hello","timestamp":"2026-05-10T12:00:00Z"}
{"type":"users","users":["Ada","Grace"]}
{"type":"typing","username":"Grace","active":true}
{"type":"system","text":"Ada joined."}
{"type":"error","error":"You are sending messages too quickly."}
```

## Limits And Safety

- Username: 2-24 characters, letters, numbers, spaces, dots, dashes, underscores.
- Room code: 4-12 uppercase letters or numbers.
- Message text: 500 characters.
- Raw WebSocket payload: 2048 bytes.
- Anti-spam cooldown: 650 ms between messages per connection.
- Max room size: 50 users.
- CORS and WebSocket origin checks use `CORS_ORIGIN`.
- Render terminates TLS, so browser clients connect with `wss://your-service.onrender.com/ws`.

## Local Setup

Install dependencies:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git
```

On macOS with Homebrew:

```bash
brew install cmake
```

Build locally:

```bash
cd backend
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

If you previously configured the project before the pinned Asio dependency was added, start from a fresh build directory:

```bash
cd backend
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Run locally:

```bash
cd backend
PORT=18080 CORS_ORIGIN=http://localhost:8000 ./build/anon_chat
```

Health check:

```bash
curl http://localhost:18080/api/health
```

Create a room:

```bash
curl -X POST http://localhost:18080/api/rooms
```

## Docker

Build the image:

```bash
cd backend
docker build -t anonymous-chat-backend .
```

Run the container:

```bash
docker run --rm -p 18080:18080 \
  -e PORT=18080 \
  -e CORS_ORIGIN=http://localhost:8000 \
  anonymous-chat-backend
```

## Render Deployment

1. Push this repository to GitHub.
2. In Render, create a new `Web Service`.
3. Choose the repository.
4. Select `Docker` as the runtime.
5. Set the root directory to `backend`.
6. Add environment variables:

```text
PORT=18080
CORS_ORIGIN=https://YOUR_GITHUB_USERNAME.github.io
```

7. Deploy.
8. Copy the Render service URL, for example `https://anonymous-chat.onrender.com`.
9. Use that URL in the frontend Connection field.

Render supports WebSockets for web services. TLS is handled by Render, so the frontend should use HTTPS for API calls and WSS for WebSockets. The frontend converts `https://` to `wss://` automatically.

## GitHub Pages Frontend Deployment

1. Commit the whole project to GitHub.
2. In the repository, open `Settings > Pages`.
3. Under `Build and deployment`, choose `Deploy from a branch`.
4. Choose the branch, then choose `/frontend` as the publish folder if available. If GitHub only offers root or `/docs`, move the frontend files to `/docs` or use a GitHub Actions Pages workflow.
5. Open the published Pages URL.
6. Expand `Connection` and enter your Render backend URL, such as `https://anonymous-chat.onrender.com`.
7. Create or join a room.

## Frontend Local Run

From the repository root:

```bash
cd frontend
python3 -m http.server 8000
```

Then open:

```text
http://localhost:8000
```

## Troubleshooting

- `WebSocket connection failed`: check that the Render URL is correct and starts with `https://`.
- `Origin not allowed`: set `CORS_ORIGIN` to the exact GitHub Pages origin, for example `https://username.github.io`.
- `Room code invalid`: use only letters and numbers, 4-12 characters.
- `You are sending messages too quickly`: wait briefly before sending another message.
- Render deploy fails during CMake: confirm Docker root directory is `backend`.
- Local build fails with `no type named 'io_service' in namespace 'asio'`: remove `backend/build` and reconfigure so CMake uses the pinned Asio 1.28.2 dependency.
- Browser blocks mixed content: GitHub Pages uses HTTPS, so the backend must also use HTTPS on Render.
- Local frontend cannot create rooms: run the backend with `CORS_ORIGIN=http://localhost:8000`.
