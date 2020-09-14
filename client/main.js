const font = "Courier New";

class CanvasManager {
  constructor(id) {
    this.canvas = document.getElementById(id);
    const fullSize = Math.min(window.innerWidth, window.innerHeight);
    this.margin = fullSize * 0.02;
    this.refSize = fullSize - 2 * this.margin;
    this.canvas.width = window.innerWidth;
    this.canvas.height = window.innerHeight - 10;
    this.ctx = this.canvas.getContext("2d");
  }

  drawLimits() {
    this.ctx.strokeStyle = "red";
    this.ctx.lineWidth = 1;
    this.ctx.beginPath();
    this.ctx.setLineDash([5, 5]);
    this.ctx.moveTo(this.margin, this.margin);
    this.ctx.lineTo(this.margin + this.refSize, this.margin);
    this.ctx.lineTo(this.margin + this.refSize, this.margin + this.refSize);
    this.ctx.lineTo(this.margin, this.margin + this.refSize);
    this.ctx.lineTo(this.margin, this.margin);
    this.ctx.stroke();
  }

  drawScore(player) {
    const pointMultiplier = 1e3;

    // draw lifetime/points
    this.ctx.textAlign = "start";
    this.ctx.font = "20px " + font;
    this.ctx.fillStyle = "black";
    let scoreStr = (player.lifetime * pointMultiplier)
      .toFixed()
      .padStart(8, " ");
    this.ctx.fillText("Score: " + scoreStr, this.margin + 10, this.margin + 25);
  }

  drawPlayer(player) {
    const accSize = (0.05 * this.refSize) / maxDd;

    const widthRect = this.refSize * player.width;
    const heightRect = this.refSize * player.height;
    const x = player.x * this.refSize;
    const y = (1 - player.y) * this.refSize;

    // draw score
    if (player.is_me) {
      this.drawScore(player);
    }

    // draw acceleration line
    this.ctx.strokeStyle = "blue";
    this.ctx.lineWidth = 1;
    this.ctx.beginPath();
    this.ctx.setLineDash([]);
    this.ctx.moveTo(this.margin + x, this.margin + y);
    this.ctx.lineTo(
      this.margin + x - accSize * player.ddx,
      this.margin + y + accSize * player.ddy
    );
    this.ctx.stroke();

    // draw player
    if (player.is_me) {
      this.ctx.fillStyle = "green";
    } else if (!player.alive) {
      this.ctx.fillStyle = "black";
    } else {
      this.ctx.fillStyle = "red";
    }

    this.ctx.beginPath();
    this.ctx.ellipse(
      this.margin + x,
      this.margin + y,
      widthRect / 2,
      heightRect / 2,
      0,
      0,
      2 * Math.PI
    );
    this.ctx.fill();
  }

  drawInputRef(x, y) {
    if (x === null || y === null) {
      return;
    }

    this.ctx.fillStyle = "rgba(0, 0, 0, 0.3)";
    this.ctx.beginPath();
    this.ctx.arc(x, y, 0.02 * this.refSize, 0, 2 * Math.PI);
    this.ctx.fill();
  }

  drawGameOver() {
    this.ctx.textAlign = "center";
    this.ctx.font = "50px " + font;
    this.ctx.fillStyle = "red";
    this.ctx.fillText(
      "GAME OVER",
      this.margin + this.refSize / 2,
      this.margin + this.refSize / 2 - 20
    );

    this.ctx.font = "20px " + font;
    this.ctx.fillStyle = "black";
    this.ctx.fillText(
      "tap to retry",
      this.margin + this.refSize / 2,
      this.margin + this.refSize / 2 + 20
    );
  }

  clear() {
    this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
  }
}

class GameEngine {
  constructor(url, canvasId) {
    this.inputRefX = null;
    this.inputRefY = null;
    this.gameIsOver = false;
    this.canvas = new CanvasManager(canvasId);
    this.sock = new WebSocket(url);
    this.sock.onopen = function () {
      this.onOpen();
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
  }

  onError(error) {
    console.error("WebSocket error", error);
  }

  onMessage(msg) {
    if (msg.game_over) {
      this.gameOver();
      return;
    }

    this.canvas.clear();
    this.canvas.drawLimits();
    this.canvas.drawInputRef(this.inputRefX, this.inputRefY);
    for (let player of msg.players) {
      this.canvas.drawPlayer(player);
    }
  }

  onInput(input) {
    if (input.control == "input_ref") {
      this.setInputRef(input.x, input.y);
      return;
    } else if (input.control == "input_ref_clear") {
      this.clearInputRef();
      return;
    }
    this.send(input);
  }

  send(msg) {
    this.sock.send(JSON.stringify(msg));
  }

  gameOver() {
    this.sock.close();
    this.canvas.drawGameOver();
    this.gameIsOver = true;
  }
}

class GameManager {
  constructor() {
    this.currentGame = null;
    const canvas = document.getElementById("canvas");
    this.input = new Input(canvas, window.innerWidth, this.onInput.bind(this));
  }

  newGame() {
    if (this.currentGame) {
      this.currentGame.gameOver();
    }
    this.currentGame = new GameEngine(
      "ws://" + window.location.hostname + ":" + window.location.port + "/ws",
      "canvas"
    );
  }

  onInput(input) {
    if (!this.currentGame) {
      return;
    }

    if (this.currentGame.gameIsOver) {
      if (input.control == "ok") {
        this.newGame();
      }
      return;
    }

    this.currentGame.onInput(input);
  }
}

function startGame() {
  console.log("Running ...");
  manager = new GameManager();
  manager.newGame();
}

// manager = null;
window.onload = startGame;
