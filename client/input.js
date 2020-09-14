const maxDd = 5;
const touchSensitivity = 5 * maxDd;
const mouseSensitivity = 25 * maxDd;

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

function clamp(v, minv, maxv) {
  if (v < minv) {
    return minv;
  } else if (v > maxv) {
    return maxv;
  } else {
    return v;
  }
}

class Input {
  constructor(element, maxDragLength, onInput) {
    this.touchStartX = 0;
    this.touchStartY = 0;
    this.mouseStartX = 0;
    this.mouseStartY = 0;
    this.mouseDown = false;
    this.onInput = onInput;
    this.maxDragLength = maxDragLength;
    element.addEventListener("touchstart", this.onTouchStart.bind(this), false);
    element.addEventListener("touchend", this.onTouchEnd.bind(this), false);
    element.addEventListener(
      "touchmove",
      this.onTouchMove.throttle(20).bind(this)
    );

    element.addEventListener("mousedown", this.onMouseDown.bind(this), false);
    element.addEventListener("mouseup", this.onMouseUp.bind(this), false);
    element.addEventListener(
      "mousemove",
      this.onMouseMove.throttle(20).bind(this)
    );
  }

  onTouchStart(e) {
    this.touchStartX = e.changedTouches[0].screenX;
    this.touchStartY = e.changedTouches[0].screenY;
  }

  onMouseDown(e) {
    this.mouseDown = true;
    this.mouseStartX = e.screenX;
    this.mouseStartY = e.screenY;
  }

  onTouchMove(e) {
    const diffX = -(e.changedTouches[0].screenX - this.touchStartX);
    const diffY = e.changedTouches[0].screenY - this.touchStartY;
    const ddx = (touchSensitivity * diffX) / this.maxDragLength;
    const ddy = (touchSensitivity * diffY) / this.maxDragLength;
    this.sendCommand(ddx, ddy);
  }

  onMouseMove(e) {
    if (!this.mouseDown) {
      return;
    }
    const diffX = -(e.screenX - this.mouseStartX);
    const diffY = e.screenY - this.mouseStartY;
    const ddx = (mouseSensitivity * diffX) / this.maxDragLength;
    const ddy = (mouseSensitivity * diffY) / this.maxDragLength;
    this.sendCommand(ddx, ddy);
  }

  onTouchEnd(_) {
    this.endCommand();
  }

  onMouseUp(_) {
    this.mouseDown = false;
    this.endCommand();
  }

  sendCommand(ddx, ddy) {
    ddx = clamp(ddx, -maxDd, maxDd);
    ddy = clamp(ddy, -maxDd, maxDd);
    this.onInput({ command: { ddx: ddx, ddy: ddy } });
  }

  endCommand() {
    this.onInput({ control: "ok", command: { ddx: 0, ddy: 0 } });
  }
}
