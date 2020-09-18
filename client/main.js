const font = "Roboto Mono";
const scoreboardSize = 8;

function createScoreboard() {
  let parent = document.getElementById("scoreboard-body");
  parent.innerHTML = "";

  for (let idx = 0; idx < scoreboardSize; ++idx) {
    let row = document.createElement("div");
    row.classList.add("scoreboard-line");

    let place = document.createElement("div");
    place.classList.add("scoreboard-place");
    place.innerText = idx + 1 + ". ";
    row.appendChild(place);

    let name = document.createElement("div");
    name.classList.add("scoreboard-name");
    name.id = "scoreboard-player-name-" + idx;
    row.appendChild(name);

    let score = document.createElement("div");
    score.classList.add("scoreboard-score");
    score.id = "scoreboard-player-score-" + idx;
    row.appendChild(score);

    parent.appendChild(row);
  }
}

function updateScoreboard(idx, name, score) {
  function update(elementId, value) {
    let element = document.getElementById(elementId);
    if (element.innerText != value) {
      element.innerText = value;
    }
  }

  update("scoreboard-player-name-" + idx, name);
  update("scoreboard-player-score-" + idx, score.toFixed());
}

class CanvasManager {
  constructor(htmlCanvas) {
    this.canvas = htmlCanvas;
    const fullSize = Math.min(this.canvas.width, this.canvas.height);

    this.margin = fullSize * 0.02;
    this.innerSize = fullSize - 2 * this.margin;
    this.ctx = this.canvas.getContext("2d");
    this.lastDrawTimes = [Date.now()];
    const smallFontSize = this.getSmallFontSize();
    this.verySmallFont = (0.65 * smallFontSize).toFixed() + "px " + font;
    this.smallFont = smallFontSize.toFixed() + "px " + font;
    this.mediumFont = (1.5 * smallFontSize).toFixed() + "px " + font;
    this.bigFont = (2.5 * smallFontSize).toFixed() + "px " + font;
  }

  getSmallFontSize() {
    // text and textSize are references, they don't matter
    // but they affect the text size
    const text = "abcdefghijklmnopqrstuvwxyz";
    const textSize = 0.5 * this.innerSize;
    let fontsize = 50;
    do {
      fontsize--;
      this.ctx.font = fontsize + "px " + font;
    } while (this.ctx.measureText(text).width > textSize);
    return fontsize;
  }

  drawTicksPerSecond() {
    this.lastDrawTimes.push(Date.now());
    this.lastDrawTimes = this.lastDrawTimes.slice(-10);
    const dt =
      this.lastDrawTimes[this.lastDrawTimes.length - 1] - this.lastDrawTimes[0];
    const tps = ((this.lastDrawTimes.length - 1) * 1000.0) / dt;
    const tpsStr = Math.round(tps).toFixed().padStart(3, " ");

    this.ctx.font = this.verySmallFont;
    this.ctx.textAlign = "end";
    this.ctx.textBaseline = "bottom";
    this.ctx.fillStyle = "white";
    this.ctx.fillText(
      "ticks/s: " + tpsStr,
      this.innerSize + this.margin - 10,
      this.innerSize + this.margin - 10
    );
  }

  drawLimits() {
    this.ctx.strokeStyle = "red";
    this.ctx.lineWidth = 1;
    this.ctx.beginPath();
    this.ctx.setLineDash([5, 5]);
    this.ctx.moveTo(this.margin, this.margin);
    this.ctx.lineTo(this.margin + this.innerSize, this.margin);
    this.ctx.lineTo(this.margin + this.innerSize, this.margin + this.innerSize);
    this.ctx.lineTo(this.margin, this.margin + this.innerSize);
    this.ctx.lineTo(this.margin, this.margin);
    this.ctx.stroke();
  }

  drawScore(player) {
    this.ctx.font = this.smallFont;
    this.ctx.textAlign = "start";
    this.ctx.textBaseline = "hanging";
    this.ctx.fillStyle = "white";
    let scoreStr = player.score.toFixed().padStart(8, " ");
    this.ctx.fillText("Score: " + scoreStr, this.margin + 10, this.margin + 10);
  }

  drawPlayer(player) {
    const accSize = (0.05 * this.innerSize) / maxDd;

    const playerSize = this.innerSize * player.size;
    const x = player.x * this.innerSize;
    const y = (1 - player.y) * this.innerSize;

    // draw score
    if (player.is_me) {
      this.drawScore(player);
    }

    // draw acceleration line
    this.ctx.strokeStyle = "rgb(150, 150, 150)";

    this.ctx.lineWidth = 1.5;
    this.ctx.beginPath();
    this.ctx.setLineDash([]);
    this.ctx.moveTo(this.margin + x, this.margin + y);
    this.ctx.lineTo(
      this.margin + x - accSize * player.ddx,
      this.margin + y + accSize * player.ddy
    );
    this.ctx.stroke();

    // draw player
    this.ctx.beginPath();
    if (player.is_me) {
      this.ctx.fillStyle = "rgb(0, 255, 0)";
    } else if (!player.alive) {
      this.ctx.fillStyle = "rgb(50, 50, 50)";
    } else if (!player.fake) {
      this.ctx.fillStyle = "rgb(255, 0, 0)";
    } else {
      this.ctx.fillStyle = "rgb(255, 100, 0)";
    }

    this.strokeStyle = "black";
    this.ctx.arc(
      this.margin + x,
      this.margin + y,
      playerSize / 2,
      0,
      2 * Math.PI
    );
    this.ctx.stroke();
    this.ctx.fill();

    this.ctx.beginPath();
    this.ctx.fillStyle = "black";
    this.ctx.arc(
      this.margin + x,
      this.margin + y,
      playerSize / 5,
      0,
      2 * Math.PI
    );
    this.ctx.fill();

    // draw player score
    this.ctx.font = this.verySmallFont;
    this.ctx.textAlign = "center";
    this.ctx.textBaseline = "hanging";
    this.ctx.fillStyle = "white";
    let scoreStr = player.score.toFixed();
    this.ctx.fillText(scoreStr, this.margin + x, this.margin + y + playerSize);
    this.ctx.textBaseline = "bottom";
    this.ctx.fillText(
      player.name,
      this.margin + x,
      this.margin + y - playerSize
    );
  }

  drawInputRef(x, y) {
    if (x === null || y === null) {
      return;
    }

    this.ctx.fillStyle = "rgba(255, 255, 255, 0.3)";
    this.ctx.beginPath();
    this.ctx.arc(x, y, 0.02 * this.innerSize, 0, 2 * Math.PI);
    this.ctx.fill();
  }

  drawGameOver() {
    this.ctx.font = this.bigFont;
    this.ctx.textAlign = "center";
    this.ctx.textBaseline = "bottom";
    this.ctx.fillStyle = "red";
    this.ctx.fillText(
      "GAME OVER",
      this.margin + this.innerSize / 2,
      this.margin + this.innerSize / 2 - 5
    );

    this.ctx.font = this.smallFont;
    this.ctx.textAlign = "center";
    this.ctx.textBaseline = "top";
    this.ctx.fillStyle = "white";
    this.ctx.fillText(
      "tap to retry",
      this.margin + this.innerSize / 2,
      this.margin + this.innerSize / 2 + 5
    );
  }

  drawWelcome() {
    this.ctx.font = this.bigFont;
    this.ctx.textAlign = "center";
    this.ctx.textBaseline = "bottom";
    this.ctx.fillStyle = "red";
    this.ctx.fillText(
      "SPACE DODGEMS",
      this.margin + this.innerSize / 2,
      this.margin + this.innerSize / 2 - 5
    );

    this.ctx.font = this.smallFont;
    this.ctx.textAlign = "center";
    this.ctx.textBaseline = "top";
    this.ctx.fillStyle = "white";
    this.ctx.fillText(
      "pick a name and tap to start",
      this.margin + this.innerSize / 2,
      this.margin + this.innerSize / 2 + 5
    );
  }

  drawError(text) {
    this.clear();
    this.ctx.font = this.mediumFont;
    this.ctx.textAlign = "center";
    this.ctx.textBaseline = "middle";
    this.ctx.fillStyle = "red";

    this.ctx.fillText(
      text.toUpperCase(),
      this.margin + this.innerSize / 2,
      this.margin + this.innerSize / 2 - 20
    );
  }

  clear() {
    this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
  }
}

class GameEngine {
  constructor(url, canvasManager, playerId, playerName) {
    this.playerId = playerId;
    this.playerName = playerName;
    this.inputRefX = null;
    this.inputRefY = null;
    this.gameIsOver = false;
    this.canvas = canvasManager;

    this.sock = new WebSocket(url);

    this.sock.onopen = function () {
      this.onOpen();
    }.bind(this);

    this.sock.onclose = function (event) {
      this.onClose(event);
    }.bind(this);

    // Log errors
    this.sock.onerror = function (error) {
      this.onError(error);
    }.bind(this);

    // Log messages from the server
    this.sock.onmessage = function (e) {
      const msg = JSON.parse(e.data);
      this.onMessage(msg);
    }.bind(this);
  }

  clearInputRef() {
    this.inputRefX = null;
    this.inputRefY = null;
  }

  setInputRef(x, y) {
    this.inputRefX = x;
    this.inputRefY = y;
  }

  onOpen() {
    console.log("Starting communication");
    this.send({
      command: { register: { id: this.playerId, name: this.playerName } },
    });
  }

  onClose(event) {
    console.log("Closing communication");
    if (event.reason) {
      this.canvas.drawError(event.reason);
    } else {
      this.canvas.drawError("CONNECTION CLOSED");
    }
  }

  onError(error) {
    console.error("WebSocket error", error);
    this.canvas.drawError("CONNECTION ERROR");
  }

  onMessage(msg) {
    if (msg.game_over) {
      this.gameOver();
      return;
    }

    this.updateScoreboard(msg.players);
    this.canvas.clear();
    this.canvas.drawTicksPerSecond();
    this.canvas.drawLimits();
    this.canvas.drawInputRef(this.inputRefX, this.inputRefY);
    for (let player of msg.players) {
      this.canvas.drawPlayer(player);
    }
  }

  onInput(input) {
    if (this.gameIsOver) {
      if (input.startInput) {
        this.restartGame();
      }
      return;
    }

    if (input.startInput) {
      this.setInputRef(input.x, input.y);
      return;
    }
    if (input.endInput) {
      this.clearInputRef();
      return;
    }

    this.send({ input: input });
  }

  send(msg) {
    this.sock.send(JSON.stringify(msg));
  }

  gameOver() {
    this.canvas.drawGameOver();
    this.gameIsOver = true;
  }

  restartGame() {
    this.gameIsOver = false;
    this.send({ command: { respawn: true } });
  }

  updateScoreboard(players) {
    let scoreboard = players.map((x) => ({
      name: x.name,
      score: x.best_score,
    }));
    scoreboard.sort((a, b) => b.score - a.score);

    const effectiveSize = Math.min(scoreboardSize, scoreboard.length);
    for (let idx = 0; idx < effectiveSize; ++idx) {
      updateScoreboard(idx, scoreboard[idx].name, scoreboard[idx].score);
    }
  }
}

class GameManager {
  constructor() {
    this.fullSize = Math.min(window.innerWidth, window.innerHeight);
    const horizontal = window.innerWidth > this.fullSize;

    this.currentGame = null;

    const canvas = document.getElementById("canvas");
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
    const spacer = document.getElementById("spacer-canvas");
    spacer.style.minWidth = this.fullSize + "px";
    spacer.style.minHeight = this.fullSize + "px";

    let container = document.getElementById("container");
    container.style.flexDirection = horizontal ? "row" : "column";

    createScoreboard();

    this.input = new Input(document, this.fullSize, this.onInput.bind(this));
    this.canvas = new CanvasManager(canvas);
    this.fillPlayerName();
    this.canvas.drawWelcome();
  }

  newGame() {
    this.input.preventDefaultTouchStart = true;
    const playerName = this.getPlayerName();
    this.currentGame = new GameEngine(
      this.getWsHref(),
      this.canvas,
      this.getPlayerId(),
      playerName
    );
    this.savePlayerName(playerName);
  }

  inputIsAValidation(input) {
    if (!input.startInput) {
      return false;
    }
    return input.x < this.fullSize && input.y < this.fullSize;
  }

  onInput(input) {
    const OPEN = 1;
    const CLOSED = 3;

    if (
      (!this.currentGame || this.currentGame.sock.readyState == CLOSED) &&
      this.inputIsAValidation(input)
    ) {
      this.newGame();
      return;
    }

    if (this.currentGame && this.currentGame.sock.readyState == OPEN) {
      this.currentGame.onInput(input);
    }
  }

  getWsHref() {
    let location = window.location.pathname.toString();
    if (location[location.length - 1] == "/") {
      location = location.substring(0, -1);
    }
    let wsHref = window.location.href.toString().replace(RegExp("^http"), "ws");
    if (wsHref[wsHref.length - 1] != "/") {
      wsHref += "/";
    }
    wsHref += "ws";
    return wsHref;
  }

  getPlayerId() {
    if (window.localStorage.playerId === undefined) {
      function uuidv4() {
        return "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx".replace(
          /[xy]/g,
          function (c) {
            var r = (Math.random() * 16) | 0,
              v = c == "x" ? r : (r & 0x3) | 0x8;
            return v.toString(16);
          }
        );
      }

      const id = uuidv4();
      console.log("new player ID: ", id);
      window.localStorage.playerId = id;
    }
    return window.localStorage.playerId;
  }

  nameIsValid(playerName) {
    return playerName.length > 0;
  }

  getPlayerName() {
    return document.getElementById("input-name").value;
  }

  savePlayerName(playerName) {
    console.log("Saving player name:", playerName);
    window.localStorage.playerName = playerName;
  }

  fillPlayerName() {
    if (window.localStorage.playerName) {
      console.log("Loading player name:", window.localStorage.playerName);
      document.getElementById("input-name").value =
        window.localStorage.playerName;
    }
  }
}

function startGame() {
  console.log("Running ...");
  manager = new GameManager();
}

window.onload = startGame;
