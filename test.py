import os
import threading
import time

import requests
from flask import Flask, jsonify, request, send_file

try:
    import serial
except ImportError:
    serial = None

ESP32_IP = "http://10.201.220.153"
BT_SERIAL_PORT = os.getenv("BT_SERIAL_PORT", "COM6")
BT_BAUD_RATE = int(os.getenv("BT_BAUD_RATE", "9600"))
SENSOR_SERIAL_PORT = os.getenv("SENSOR_SERIAL_PORT", os.getenv("BT_SERIAL_PORT", "COM11"))
SENSOR_SERIAL_BAUD = int(os.getenv("SENSOR_SERIAL_BAUD", "115200"))

COMMAND_MAP = {
    "f": "forward",
    "b": "back",
    "l": "left",
    "r": "right",
    "s": "stop",
    "forward": "forward",
    "back": "back",
    "left": "left",
    "right": "right",
    "stop": "stop",
}

BT_COMMAND_MAP = {
    "forward": "f",
    "back": "b",
    "left": "l",
    "right": "r",
    "stop": "s",
}

TRANSPORTS = {"wifi", "bluetooth", "both"}

app = Flask(__name__)

sensor_lock = threading.Lock()
sensor_status = {
    "detected": False,
    "headline": "No human obstacle detected",
    "detail": "Waiting for sensor data...",
    "raw": "",
    "updated_at": None,
}


def set_sensor_status(detected, headline, detail, raw):
    with sensor_lock:
        sensor_status.update(
            {
                "detected": detected,
                "headline": headline,
                "detail": detail,
                "raw": raw,
                "updated_at": time.time(),
            }
        )


def parse_sensor_line(line):
    text = line.strip()
    lowered = text.lower()

    if not text:
        return None

    if "obstacle detected" in lowered or "object detected" in lowered:
        return True, "Human action detected", text

    if lowered.startswith("ir:") and ("0" in lowered or "low" in lowered):
        return True, "Human action detected", text

    if "all normal" in lowered or "no object" in lowered:
        return False, "No human obstacle detected", text

    return None


def sensor_monitor():
    if serial is None:
        set_sensor_status(False, "Sensor feed unavailable", "pyserial is not installed.", "")
        return

    try:
        with serial.Serial(SENSOR_SERIAL_PORT, SENSOR_SERIAL_BAUD, timeout=1) as conn:
            time.sleep(2)
            set_sensor_status(False, "Sensor feed active", f"Listening on {SENSOR_SERIAL_PORT}", "")

            while True:
                raw_line = conn.readline().decode("utf-8", errors="ignore")
                parsed = parse_sensor_line(raw_line)
                if parsed is None:
                    continue

                detected, headline, detail = parsed
                set_sensor_status(detected, headline, detail, raw_line.strip())
    except Exception as exc:
        set_sensor_status(False, "Sensor feed unavailable", f"{SENSOR_SERIAL_PORT}: {exc}", "")


def start_sensor_monitor():
    thread = threading.Thread(target=sensor_monitor, daemon=True)
    thread.start()


start_sensor_monitor()


def send_wifi(command):
    url = f"{ESP32_IP}/{command}"
    response = requests.get(url, timeout=5)
    response.raise_for_status()
    return response.text


def send_bluetooth(command):
    if serial is None:
        raise RuntimeError("pyserial is not installed. Run: pip install pyserial")

    bt_code = BT_COMMAND_MAP[command]
    with serial.Serial(BT_SERIAL_PORT, BT_BAUD_RATE, timeout=1) as conn:
        conn.write(f"{bt_code}\n".encode("ascii"))
        conn.flush()
    return f"Sent '{bt_code}' to {BT_SERIAL_PORT} at {BT_BAUD_RATE} baud"


@app.get("/")
def index():
    return send_file("frontend.html")


@app.post("/api/command")
def command():
    payload = request.get_json(silent=True) or {}
    raw_command = str(payload.get("command", "")).strip().lower()
    transport = str(payload.get("transport", "wifi")).strip().lower()
    mapped = COMMAND_MAP.get(raw_command)

    if not mapped:
        return jsonify({"ok": False, "error": "Invalid command"}), 400
    if transport not in TRANSPORTS:
        return jsonify({"ok": False, "error": "Invalid transport. Use wifi, bluetooth, or both."}), 400

    try:
        result = {"ok": True, "command": mapped, "transport": transport}

        if transport in {"wifi", "both"}:
            result["wifi_response"] = send_wifi(mapped)

        if transport in {"bluetooth", "both"}:
            result["bluetooth_response"] = send_bluetooth(mapped)

        return jsonify(result)
    except requests.RequestException as exc:
        return jsonify({"ok": False, "error": f"Wi-Fi error connecting to ESP32: {exc}"}), 502
    except Exception as exc:
        return jsonify({"ok": False, "error": f"Bluetooth error: {exc}"}), 502


@app.get("/api/status")
def status():
    with sensor_lock:
        return jsonify(sensor_status)


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)