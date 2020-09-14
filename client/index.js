class CanvasManager {
  constructor(id) {
    this.canvas = document.getElementById(id);
    let size = Math.min(window.innerWidth, window.innerHeight);
    this.canvas.width = size * 0.95;
    this.canvas.height = size * 0.95;
    this.ctx = this.canvas.getContext("2d");
  }

  drawPlayer(player) {
    const w = 30;
    const h = 30;

    let x = player.x * this.canvas.width;
    let y = (1 - player.y) * this.canvas.height;

    if (player.is_me) {
      this.ctx.fillStyle = "green";
    } else if (!player.alive) {
      this.ctx.fillStyle = "black";
    } else {
      this.ctx.fillStyle = "red";
    }

    this.ctx.fillRect(x - w / 2, y - h / 2, w, h);
  }

  drawGameOver() {
    this.ctx.textAlign = "center";
    this.ctx.font = "50px Verdana";
    this.ctx.fillStyle = "red";
    this.ctx.fillText(
      "GAME OVER",
      this.canvas.width / 2,
      this.canvas.height / 2 - 20
    );

    this.ctx.font = "20px Verdana";
    this.ctx.fillStyle = "black";
    this.ctx.fillText(
      "press space to retry",
      this.canvas.width / 2,
      this.canvas.height / 2 + 20
    );
  }

  clear() {
    this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
  }
}

class GameEngine {
  constructor(url, canvasId) {
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

  onOpen() {
    console.log("Starting communication");
  }

  onError(error) {
    console.error("WebSocket error", error);
  }

  onMessage(msg) {
    this.canvas.clear();

    if (msg.game_over) {
      this.gameOver();
      return;
    }

    console.log("state:", msg);
    for (let player of msg.players) {
      this.canvas.drawPlayer(player);
    }
  }

  onKeyDown(event) {
    if (event.keyCode == 37) {
      this.send({ key: "left" });
    } else if (event.keyCode == 39) {
      this.send({ key: "right" });
    } else if (event.keyCode == 38) {
      this.send({ key: "up" });
    } else if (event.keyCode == 40) {
      this.send({ key: "down" });
    }
  }

  send(msg) {
    console.log("sending", msg);
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

    document.addEventListener("keydown", this.onKeyDown.bind(this));
  }

  newGame() {
    if (this.currentGame) {
      this.currentGame.gameOver();
    }
    this.currentGame = new GameEngine("ws://localhost:8080/ws", "canvas");
  }

  onKeyDown(event) {
    if (!this.currentGame) {
      return;
    }

    if (this.currentGame.gameIsOver) {
      if (event.keyCode == 32) {
        // space
        this.newGame();
      }
    } else {
      this.currentGame.onKeyDown(event);
    }
  }
}

function startGame() {
  console.log("Running ...");
  manager.newGame();
}

manager = new GameManager();
window.onload = startGame;
