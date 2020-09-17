const maxDd = 5;
const touchSensitivity = 5 * maxDd;
const mouseSensitivity = 5 * maxDd;
const inputFrequency = 30;

Function.prototype.throttle = function (milliseconds, context) {
  var baseFunction = this,
    lastEventTimestamp = null,
    limit = milliseconds;

  return function () {
    var self = context || this,
      args = arguments,
      now = Date.now();

    if (!lastEventTimestamp || now - lastEventTimestamp >= limit) {
      lastEventTimestamp = now;
      baseFunction.apply(self, args);
    }
  };
};

class Input {
  constructor(element, maxDragLength, onInput) {
    this.touchStartX = 0;
    this.touchStartY = 0;
    this.mouseStartX = 0;
    this.mouseStartY = 0;
    this.mouseDown = false;
    this.onInput = onInput;
    this.maxDragLength = maxDragLength;
    this.preventDefaultTouchStart = false;

    element.addEventListener("touchstart", this.onTouchStart.bind(this), {
      passive: false,
    });
    element.addEventListener("touchend", this.onTouchEnd.bind(this));
    element.addEventListener(
      "touchmove",
      this.onTouchMove.throttle(1.0 / inputFrequency).bind(this)
    );

    element.addEventListener("mousedown", this.onMouseDown.bind(this));
    element.addEventListener("mouseup", this.onMouseUp.bind(this));
    element.addEventListener(
      "mousemove",
      this.onMouseMove.throttle(1.0 / inputFrequency).bind(this)
    );
  }

  onTouchStart(e) {
    if (this.preventDefaultTouchStart) {
      // preventing the touchstart will prevent
      // chrome from throttling touchmove on mobile
      // but will also prevent the "input" field
      // from working
      e.preventDefault();
    }
    this.touchStartX = e.changedTouches[0].clientX;
    this.touchStartY = e.changedTouches[0].clientY;
    this.onInput({
      startInput: true,
      x: this.touchStartX,
      y: this.touchStartY,
    });
  }

  onMouseDown(e) {
    this.mouseDown = true;
    this.mouseStartX = e.clientX;
    this.mouseStartY = e.clientY;
    this.onInput({
      startInput: true,
      x: this.mouseStartX,
      y: this.mouseStartY,
    });
  }

  onTouchMove(e) {
    const diffX = -(e.changedTouches[0].clientX - this.touchStartX);
    const diffY = e.changedTouches[0].clientY - this.touchStartY;
    const ddx = (touchSensitivity * diffX) / this.maxDragLength;
    const ddy = (touchSensitivity * diffY) / this.maxDragLength;
    this.sendCommand(ddx, ddy);
  }

  onMouseMove(e) {
    if (!this.mouseDown) {
      return;
    }
    const diffX = -(e.clientX - this.mouseStartX);
    const diffY = e.clientY - this.mouseStartY;
    const ddx = (mouseSensitivity * diffX) / this.maxDragLength;
    const ddy = (mouseSensitivity * diffY) / this.maxDragLength;
    this.sendCommand(ddx, ddy);
  }

  onTouchEnd(_) {
    this.onInput({
      endInput: true,
    });
    this.endCommand();
  }

  onMouseUp(_) {
    this.onInput({
      endInput: true,
    });
    this.mouseDown = false;
    this.endCommand();
  }

  sendCommand(ddx, ddy) {
    this.onInput({ ddx: ddx, ddy: ddy });
  }

  endCommand() {
    this.onInput({ ddx: 0, ddy: 0 });
  }
}
