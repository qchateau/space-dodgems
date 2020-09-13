class CanvasManager {
  constructor(id) {
    this.canvas = document.getElementById(id);
    this.canvas.width = window.innerWidth * 0.9;
    this.canvas.height = window.innerHeight * 0.9;
    this.ctx = this.canvas.getContext("2d");
  }

  drawRect(x, y) {
    const w = 100;
    const h = 100;

    // bring origin to the middle
    y = this.canvas.height / 2 - y;
    x = this.canvas.width / 2 + x;

    this.ctx.fillStyle = "green";
    this.ctx.fillRect(x - w / 2, y - h / 2, w, h);
  }

  clear() {
    this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
  }
}

class Engine {
  constructor(url, canvasId) {
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

    // Listen keyboard inputs
    document.addEventListener("keydown", this.onKeyDown.bind(this));
  }

  onOpen() {
    console.log("Starting communication");
  }

  onError(error) {
    console.error("WebSocket error", error);
  }

  onMessage(msg) {
    this.canvas.clear();
    for (let player of msg.players) {
      this.canvas.drawRect(player.x, player.y);
    }
  }

  onKeyDown(event) {
    console.log("key press", event.keyCode);
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
}

function main() {
  console.log("Running ...");
  engine = new Engine("ws://localhost:8080/ws", "canvas");
}

window.onload = main;
