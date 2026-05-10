const MAX_MESSAGE_LENGTH = 500;
const DEFAULT_REMOTE_BACKEND = "https://your-render-service.onrender.com";

const state = {
  ws: null,
  username: "",
  roomCode: "",
  users: [],
  typingUsers: new Set(),
  typingTimer: null,
  reconnecting: false
};

const els = {
  landing: document.querySelector("#landing"),
  chat: document.querySelector("#chat"),
  joinForm: document.querySelector("#joinForm"),
  createRoomBtn: document.querySelector("#createRoomBtn"),
  username: document.querySelector("#username"),
  roomCode: document.querySelector("#roomCode"),
  backendUrl: document.querySelector("#backendUrl"),
  landingError: document.querySelector("#landingError"),
  activeRoom: document.querySelector("#activeRoom"),
  activeName: document.querySelector("#activeName"),
  leaveBtn: document.querySelector("#leaveBtn"),
  usersList: document.querySelector("#usersList"),
  userCount: document.querySelector("#userCount"),
  messages: document.querySelector("#messages"),
  typingIndicator: document.querySelector("#typingIndicator"),
  messageForm: document.querySelector("#messageForm"),
  messageInput: document.querySelector("#messageInput"),
  messageTemplate: document.querySelector("#messageTemplate")
};

init();

function init() {
  const savedBackend = localStorage.getItem("chatBackendUrl");
  els.backendUrl.value = savedBackend || inferBackendUrl();

  els.createRoomBtn.addEventListener("click", createRoom);
  els.joinForm.addEventListener("submit", (event) => {
    event.preventDefault();
    joinRoom(els.roomCode.value.trim().toUpperCase());
  });
  els.leaveBtn.addEventListener("click", leaveRoom);
  els.messageForm.addEventListener("submit", sendMessage);
  els.messageInput.addEventListener("input", sendTyping);
}

function inferBackendUrl() {
  if (location.hostname === "localhost" || location.hostname === "127.0.0.1") {
    return "http://localhost:18080";
  }
  return DEFAULT_REMOTE_BACKEND;
}

function getBackendUrl() {
  const url = els.backendUrl.value.trim().replace(/\/+$/, "");
  if (!url || url === DEFAULT_REMOTE_BACKEND) {
    throw new Error("Set your Render backend URL in Connection.");
  }
  localStorage.setItem("chatBackendUrl", url);
  return url;
}

function getWsUrl(baseUrl, roomCode, username) {
  const url = new URL(baseUrl);
  url.protocol = url.protocol === "https:" ? "wss:" : "ws:";
  url.pathname = "/ws";
  url.search = new URLSearchParams({ room: roomCode, username }).toString();
  return url.toString();
}

async function createRoom() {
  try {
    setLandingError("");
    const username = cleanUsername();
    const backendUrl = getBackendUrl();
    const response = await fetch(`${backendUrl}/api/rooms`, { method: "POST" });
    const data = await response.json();
    if (!response.ok) {
      throw new Error(data.error || "Could not create room.");
    }
    els.roomCode.value = data.roomCode;
    joinRoom(data.roomCode, username);
  } catch (error) {
    setLandingError(error.message);
  }
}

function joinRoom(roomCode, providedUsername) {
  try {
    setLandingError("");
    const username = providedUsername || cleanUsername();
    const normalizedRoom = cleanRoomCode(roomCode);
    const backendUrl = getBackendUrl();

    state.username = username;
    state.roomCode = normalizedRoom;
    state.typingUsers.clear();

    const ws = new WebSocket(getWsUrl(backendUrl, normalizedRoom, username));
    state.ws = ws;

    ws.addEventListener("open", () => {
      showChat();
      appendSystemMessage(`Joined room ${normalizedRoom}.`);
    });

    ws.addEventListener("message", (event) => handleSocketMessage(event.data));
    ws.addEventListener("close", (event) => handleSocketClose(event));
    ws.addEventListener("error", () => setLandingError("WebSocket connection failed."));
  } catch (error) {
    setLandingError(error.message);
  }
}

function cleanUsername() {
  const username = els.username.value.trim();
  if (!/^[a-zA-Z0-9 _.-]{2,24}$/.test(username)) {
    throw new Error("Use a username with 2-24 letters, numbers, spaces, dots, dashes, or underscores.");
  }
  return username;
}

function cleanRoomCode(roomCode) {
  const normalizedRoom = roomCode.trim().toUpperCase();
  if (!/^[A-Z0-9]{4,12}$/.test(normalizedRoom)) {
    throw new Error("Use a room code with 4-12 letters or numbers.");
  }
  return normalizedRoom;
}

function handleSocketMessage(rawMessage) {
  let data;
  try {
    data = JSON.parse(rawMessage);
  } catch {
    return;
  }

  if (data.type === "message") {
    appendMessage(data);
  } else if (data.type === "system") {
    appendSystemMessage(data.text);
  } else if (data.type === "users") {
    state.users = Array.isArray(data.users) ? data.users : [];
    renderUsers();
  } else if (data.type === "typing") {
    handleTyping(data);
  } else if (data.type === "error") {
    appendSystemMessage(data.error || "Something went wrong.");
  }
}

function handleSocketClose(event) {
  if (state.reconnecting) {
    return;
  }
  state.ws = null;
  appendSystemMessage(event.reason || "Disconnected.");
  els.messageInput.disabled = true;
}

function showChat() {
  els.landing.classList.add("hidden");
  els.chat.classList.remove("hidden");
  els.activeRoom.textContent = state.roomCode;
  els.activeName.textContent = state.username;
  els.messages.textContent = "";
  els.messageInput.disabled = false;
  els.messageInput.focus();
}

function leaveRoom() {
  state.reconnecting = true;
  if (state.ws) {
    state.ws.close(1000, "Leaving room");
  }
  state.ws = null;
  state.users = [];
  state.typingUsers.clear();
  renderUsers();
  updateTypingIndicator();
  els.chat.classList.add("hidden");
  els.landing.classList.remove("hidden");
  els.messageInput.value = "";
  state.reconnecting = false;
}

function sendMessage(event) {
  event.preventDefault();
  const text = els.messageInput.value.trim();
  if (!text || !state.ws || state.ws.readyState !== WebSocket.OPEN) {
    return;
  }
  if (text.length > MAX_MESSAGE_LENGTH) {
    appendSystemMessage(`Messages are limited to ${MAX_MESSAGE_LENGTH} characters.`);
    return;
  }
  state.ws.send(JSON.stringify({ type: "message", text }));
  els.messageInput.value = "";
  state.ws.send(JSON.stringify({ type: "typing", active: false }));
}

function sendTyping() {
  if (!state.ws || state.ws.readyState !== WebSocket.OPEN) {
    return;
  }
  state.ws.send(JSON.stringify({ type: "typing", active: true }));
  clearTimeout(state.typingTimer);
  state.typingTimer = setTimeout(() => {
    if (state.ws && state.ws.readyState === WebSocket.OPEN) {
      state.ws.send(JSON.stringify({ type: "typing", active: false }));
    }
  }, 900);
}

function handleTyping(data) {
  if (!data.username || data.username === state.username) {
    return;
  }
  if (data.active) {
    state.typingUsers.add(data.username);
  } else {
    state.typingUsers.delete(data.username);
  }
  updateTypingIndicator();
}

function appendMessage(data) {
  const node = els.messageTemplate.content.firstElementChild.cloneNode(true);
  node.classList.toggle("own", data.username === state.username);
  node.querySelector(".message-user").textContent = data.username || "Anonymous";
  node.querySelector(".message-time").textContent = formatTime(data.timestamp);
  node.querySelector(".message-text").textContent = data.text || "";
  els.messages.appendChild(node);
  scrollMessages();
}

function appendSystemMessage(text) {
  const node = document.createElement("article");
  node.className = "message system";
  node.textContent = text;
  els.messages.appendChild(node);
  scrollMessages();
}

function renderUsers() {
  els.usersList.textContent = "";
  els.userCount.textContent = String(state.users.length);
  for (const user of state.users) {
    const li = document.createElement("li");
    li.textContent = user;
    els.usersList.appendChild(li);
  }
}

function updateTypingIndicator() {
  const users = [...state.typingUsers];
  if (!users.length) {
    els.typingIndicator.textContent = "";
  } else if (users.length === 1) {
    els.typingIndicator.textContent = `${users[0]} is typing...`;
  } else {
    els.typingIndicator.textContent = `${users.slice(0, 2).join(", ")} are typing...`;
  }
}

function formatTime(timestamp) {
  const date = timestamp ? new Date(timestamp) : new Date();
  return date.toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" });
}

function scrollMessages() {
  els.messages.scrollTop = els.messages.scrollHeight;
}

function setLandingError(message) {
  els.landingError.textContent = message;
}
