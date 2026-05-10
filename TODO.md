# Anonymous Realtime Chat TODO

You are currently at: **Backend local build**

## 1. Clean And Rebuild Backend Locally

- [ ] Open a terminal.
- [ ] Go to the backend folder:

```bash
cd /Users/maryansaitej/Documents/Codex/2026-05-10/you-are-a-senior-full-stack/backend
```

- [ ] Remove the old build cache. This is required because the previous cache pointed to the wrong Asio include path:

```bash
rm -rf build
```

- [ ] Configure the CMake project:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

- [ ] Build the backend:

```bash
cmake --build build --parallel
```

- [ ] Confirm the backend binary exists:

```bash
ls build/anon_chat
```

## 2. Run Backend Locally

- [ ] Start the backend on port `18080`:

```bash
PORT=18080 CORS_ORIGIN=http://localhost:8000 ./build/anon_chat
```

- [ ] Leave this terminal running.
- [ ] Open a second terminal for frontend work.

## 3. Test Backend Health

- [ ] In the second terminal, run:

```bash
curl http://localhost:18080/api/health
```

- [ ] Expected result:

```json
{"status":"ok"}
```

- [ ] Create a test room:

```bash
curl -X POST http://localhost:18080/api/rooms
```

- [ ] Expected result looks like:

```json
{"roomCode":"ABC123"}
```

## 4. Run Frontend Locally

- [ ] Go to the frontend folder:

```bash
cd /Users/maryansaitej/Documents/Codex/2026-05-10/you-are-a-senior-full-stack/frontend
```

- [ ] Start a static server:

```bash
python3 -m http.server 8000
```

- [ ] Open this URL in your browser:

```text
http://localhost:8000
```

## 5. Test End-To-End Locally

- [ ] In the frontend, enter a username.
- [ ] Expand `Connection`.
- [ ] Confirm backend URL is:

```text
http://localhost:18080
```

- [ ] Click `Create room`.
- [ ] Open a second browser tab at:

```text
http://localhost:8000
```

- [ ] Use a different username.
- [ ] Enter the same room code.
- [ ] Click `Join room`.
- [ ] Send messages from both tabs.
- [ ] Confirm:
  - messages appear in realtime
  - online users list updates
  - typing indicator appears
  - leave button disconnects users
  - room disappears when all users leave

## 6. If Local Build Fails

- [ ] If you see `asio.hpp file not found`, make sure you removed the old build folder:

```bash
cd /Users/maryansaitej/Documents/Codex/2026-05-10/you-are-a-senior-full-stack/backend
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

- [ ] If CMake cannot download dependencies, check internet access and retry.
- [ ] If Crow errors mention WebSocket API signatures, confirm `src/main.cpp` contains `.websocket(&app)`.
- [ ] If the backend runs but frontend cannot connect, confirm the backend terminal is still running.
- [ ] If the browser blocks the request, confirm `CORS_ORIGIN=http://localhost:8000` was set when starting the backend.

## 7. Docker Build

- [ ] Build the Docker image:

```bash
cd /Users/maryansaitej/Documents/Codex/2026-05-10/you-are-a-senior-full-stack/backend
docker build -t anonymous-chat-backend .
```

- [ ] Run the Docker container:

```bash
docker run --rm -p 18080:18080 \
  -e PORT=18080 \
  -e CORS_ORIGIN=http://localhost:8000 \
  anonymous-chat-backend
```

- [ ] Test health again:

```bash
curl http://localhost:18080/api/health
```

## 8. Push Project To GitHub

- [ ] Create a GitHub repository.
- [ ] Commit the project.
- [ ] Push the project to GitHub.

Suggested commands:

```bash
cd /Users/maryansaitej/Documents/Codex/2026-05-10/you-are-a-senior-full-stack
git init
git add frontend backend TODO.md
git commit -m "Build anonymous realtime chat app"
git branch -M main
git remote add origin YOUR_REPOSITORY_URL
git push -u origin main
```

## 9. Deploy Backend To Render

- [ ] In Render, create a new `Web Service`.
- [ ] Connect the GitHub repository.
- [ ] Choose `Docker` runtime.
- [ ] Set root directory to:

```text
backend
```

- [ ] Add environment variables:

```text
PORT=18080
CORS_ORIGIN=https://YOUR_GITHUB_USERNAME.github.io
```

- [ ] Deploy the service.
- [ ] Copy the Render URL, for example:

```text
https://anonymous-chat.onrender.com
```

- [ ] Test the deployed backend:

```bash
curl https://YOUR_RENDER_SERVICE.onrender.com/api/health
```

## 10. Deploy Frontend To GitHub Pages

- [ ] Go to GitHub repository settings.
- [ ] Open `Pages`.
- [ ] Choose deployment from branch.
- [ ] Select the main branch.
- [ ] Publish the `frontend` folder if GitHub offers it.
- [ ] If GitHub does not offer `frontend`, move frontend files to `docs/` or use a GitHub Actions Pages workflow.
- [ ] Open the GitHub Pages URL.
- [ ] Expand `Connection`.
- [ ] Enter your Render backend URL:

```text
https://YOUR_RENDER_SERVICE.onrender.com
```

- [ ] Create and join a room.

## 11. Production Smoke Test

- [ ] Open the GitHub Pages site in two browser tabs.
- [ ] Use two different usernames.
- [ ] Create a room in one tab.
- [ ] Join the same room in the other tab.
- [ ] Send messages both ways.
- [ ] Confirm `wss://` WebSocket connection works.
- [ ] Confirm users list updates.
- [ ] Confirm typing indicator works.
- [ ] Confirm leaving removes users.

## 12. Final Cleanup

- [ ] Replace placeholder text like `YOUR_RENDER_SERVICE` and `YOUR_GITHUB_USERNAME` in any personal notes.
- [ ] Confirm `backend/README.md` is accurate for your actual deployed URLs.
- [ ] Confirm no secrets are committed. This app should not need any secrets.
- [ ] Keep Render backend URL handy for frontend users.
