class CanvasManager {
  constructor(id) {
    this.canvas = document.getElementById(id);
    this.canvas.width = window.innerWidth;
    this.canvas.height = window.innerHeight;
    this.ctx = this.canvas.getContext("2d");
  }

  drawRect(x, y) {
    const w = 100;
    const h = 100;

    this.clear();
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
  }

  onOpen() {
    console.log("Starting communication");
  }

  onError(error) {
    console.error("WebSocket error", error);
  }

  onMessage(msg) {
    console.log(msg);
    this.canvas.drawRect(msg.x, msg.y);
  }
}

function main() {
  console.log("Running ...");
  engine = new Engine("ws://localhost:8080/ws", "canvas");
}

window.onload = main;
