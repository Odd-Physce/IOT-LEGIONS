import time
import json
import threading
import subprocess
import sys
import os
import serial
import requests

from flask import Flask, render_template
from flask_socketio import SocketIO

# -------- FLASK SETUP --------
app = Flask(__name__)
app.config["TEMPLATES_AUTO_RELOAD"] = True
app.jinja_env.auto_reload = True

# 🔥 IMPORTANT: force threading mode (stable)
socketio = SocketIO(app, cors_allowed_origins="*", async_mode="threading")

# -------- SERIAL CONFIG --------
SERIAL_PORT = "COM6"
BAUD_RATE = 115200

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    print(f"✅ Serial connected on {SERIAL_PORT}")
except Exception as e:
    print(f"❌ Serial connection failed: {e}")
    ser = None

# -------- SMS CONFIG --------
PHONE_NUMBER = "9036959557"
CIRCUITDIGEST_API_KEY = os.getenv("CIRCUITDIGEST_API_KEY", "")
SMS_TEMPLATE_ID = "103"  # Replace with your approved template ID from CircuitDigest.
NODE_ALERT_COOLDOWN = 15
BOTH_FAIL_ALERT_COOLDOWN = 30
last_node_alert_time = 0
last_both_fail_alert_time = 0

# -------- FAILOVER APP STATE --------
node_last_label = {1: None, 2: None}
sr_opened_for_current_fail = False
sr_process = None


def launch_sr_app():
    global sr_process
    try:
        sr_path = os.path.join(os.path.dirname(__file__), "sr.py")

        # Avoid spawning duplicate windows while one sr.py process is still alive.
        if sr_process is not None and sr_process.poll() is None:
            print("ℹ️ sr.py already running")
            return

        log_path = os.path.join(os.path.dirname(__file__), "sr_launch.log")
        log_file = open(log_path, "a", encoding="utf-8")

        creation_flags = 0
        if os.name == "nt":
            creation_flags = getattr(subprocess, "CREATE_NEW_CONSOLE", 0)

        sr_process = subprocess.Popen(
            [sys.executable, sr_path],
            cwd=os.path.dirname(__file__),
            stdout=log_file,
            stderr=log_file,
            creationflags=creation_flags,
        )
        print(f"🚀 Opened sr.py (pid={sr_process.pid}) because both nodes reached structure_fail")
        print(f"📝 sr.py output log: {log_path}")
    except Exception as e:
        print(f"❌ Failed to open sr.py: {e}")

# -------- SMS FUNCTION --------
def send_sms(message):
    try:
        if not CIRCUITDIGEST_API_KEY:
            print("SMS Failed: CIRCUITDIGEST_API_KEY is not set")
            return

        # CircuitDigest SMS API uses template variables (var1, var2, ...).
        url = f"https://www.circuitdigest.cloud/api/v1/send_sms?ID={SMS_TEMPLATE_ID}"

        payload = {
            "mobiles": PHONE_NUMBER,
            "var1": str(message),
            "var2": "Vibration Monitor"
        }

        headers = {
            "Content-Type": "application/json",
            "Authorization": CIRCUITDIGEST_API_KEY,
        }

        response = requests.post(url, json=payload, headers=headers, timeout=10)
        print(f"SMS API status={response.status_code}, response={response.text}")

    except Exception as e:
        print("SMS Failed:", e)


def send_node_alert_sms(node, label, confidence, sample_time):
    msg = (
        f"ALERT Node {node}: {label} | "
        f"conf={confidence:.4f} | t={sample_time}"
    )
    send_sms(msg)


def send_both_fail_sms():
    msg = "EMERGENCY: Both Node 1 and Node 2 reached structure_fail"
    send_sms(msg)

# -------- ROUTE --------
@app.route("/")
def index():
    build = time.strftime("%Y-%m-%d %H:%M:%S")
    return render_template("index.html", build=build)


@socketio.on("connect")
def handle_connect():
    print("🔌 Browser connected")
    socketio.emit("server_status", {"ok": True, "message": "socket connected"})

# -------- SERIAL READER --------
def read_serial():
    global last_node_alert_time, last_both_fail_alert_time, sr_opened_for_current_fail

    if ser is None:
        print("❌ Serial not available, cannot read data")
        return

    print("🔄 Reading Serial Data...")

    while True:
        try:
            line = ser.readline().decode(errors="ignore").strip()

            if not line or not line.startswith("{"):
                continue

            data = json.loads(line)

            # ✅ VALIDATE DATA STRUCTURE
            if not all(key in data for key in ["node", "label", "az"]):
                print("⚠️ Invalid data structure:", data)
                continue

            print("✅ DATA:", data)

            # Track latest label per node and launch sr.py when both are structure_fail.
            raw_node = data.get("node")
            raw_label = data.get("label", "")
            confidence = float(data.get("confidence", 0.0))
            sample_time = data.get("time", "NA")

            try:
                node = int(raw_node)
            except (TypeError, ValueError):
                node = None

            label = str(raw_label).strip().lower().replace("-", "_").replace(" ", "_")

            if node in (1, 2):
                node_last_label[node] = label

            print(f"🧭 Fail state: node1={node_last_label.get(1)} node2={node_last_label.get(2)}")

            both_failed = (
                node_last_label.get(1) == "structure_fail"
                and node_last_label.get(2) == "structure_fail"
            )

            if both_failed and not sr_opened_for_current_fail:
                launch_sr_app()
                sr_opened_for_current_fail = True
                print("✅ Both nodes are structure_fail -> trigger fired")

                current_time = time.time()
                if current_time - last_both_fail_alert_time > BOTH_FAIL_ALERT_COOLDOWN:
                    send_both_fail_sms()
                    last_both_fail_alert_time = current_time
            elif not both_failed:
                sr_opened_for_current_fail = False

            # 🔥 SEND DATA TO FRONTEND
            socketio.emit("sensor_data", data)

            # 🚨 SMS ALERT
            current_time = time.time()

            if (
                label in ["critical", "structure_fail"]
                and current_time - last_node_alert_time > NODE_ALERT_COOLDOWN
            ):
                send_node_alert_sms(node, label, confidence, sample_time)
                last_node_alert_time = current_time

        except json.JSONDecodeError as e:
            print(f"⚠️ JSON Parse Error: {e}")
        except Exception as e:
            print(f"❌ Serial Error: {e}")

# -------- MAIN --------
if __name__ == "__main__":
    print("🔥 Server Started")

    threading.Thread(target=read_serial, daemon=True).start()

    print("🌐 http://127.0.0.1:5000")

    socketio.run(app, host="0.0.0.0", port=5000, allow_unsafe_werkzeug=True)