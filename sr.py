import socket
import threading
import tkinter as tk
import cv2
from PIL import Image, ImageTk

# -------- SETTINGS --------
ESP32_IP = "10.201.220.123"   # 🔥 put your ESP IP here
SEND_PORT = 4210
RECV_PORT = 4211

# 🔥 PUT YOUR PHONE IP HERE
CAMERA_URL = "http://100.75.99.135:8080/video"

class RobotApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Robot Dashboard")

        self.last_cmd = ""
        self.running = True

        # -------- UDP --------
        self.send_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        self.recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.recv_sock.bind(("", RECV_PORT))

        # -------- UI --------
        self.sensor_text = tk.StringVar(value="No Data")

        self.build_ui()

        # -------- CAMERA (ONLY IP CAM) --------
        self.cap = cv2.VideoCapture(CAMERA_URL, cv2.CAP_FFMPEG)

        if not self.cap.isOpened():
            print("❌ Camera not connected — check IP")

        # -------- THREAD --------
        threading.Thread(target=self.sensor_thread, daemon=True).start()
        self.update_camera()

        # -------- KEYBOARD --------
        self.bind_keys()

    # -------- UI --------
    def build_ui(self):
        main = tk.Frame(self.root)
        main.pack()

        self.camera_label = tk.Label(main, text="Connecting Camera...")
        self.camera_label.pack()

        tk.Label(main, textvariable=self.sensor_text).pack(pady=10)

        btn_frame = tk.Frame(main)
        btn_frame.pack()

        self.btn_f = tk.Button(btn_frame, text="Forward")
        self.btn_l = tk.Button(btn_frame, text="Left")
        self.btn_s = tk.Button(btn_frame, text="Stop", command=lambda: self.send('s'))
        self.btn_r = tk.Button(btn_frame, text="Right")
        self.btn_b = tk.Button(btn_frame, text="Back")

        self.btn_f.grid(row=0, column=1)
        self.btn_l.grid(row=1, column=0)
        self.btn_s.grid(row=1, column=1)
        self.btn_r.grid(row=1, column=2)
        self.btn_b.grid(row=2, column=1)

        # Hold control
        self.bind_hold(self.btn_f, 'f')
        self.bind_hold(self.btn_b, 'b')
        self.bind_hold(self.btn_l, 'l')
        self.bind_hold(self.btn_r, 'r')

    # -------- BUTTON HOLD --------
    def bind_hold(self, btn, cmd):
        btn.bind("<ButtonPress-1>", lambda e: self.send(cmd))
        btn.bind("<ButtonRelease-1>", lambda e: self.send('s'))

    # -------- SEND --------
    def send(self, cmd):
        if cmd == self.last_cmd:
            return

        print("Sending:", cmd)

        try:
            self.send_sock.sendto(cmd.encode(), (ESP32_IP, SEND_PORT))
            self.last_cmd = cmd
        except:
            print("Send error")

    # -------- SENSOR --------
    def sensor_thread(self):
        while True:
            try:
                data, _ = self.recv_sock.recvfrom(1024)
                ir, flame, gas, dist = data.decode().split(",")

                text = (
                    f"IR: {'🚧 Object' if ir=='0' else 'Clear'} | "
                    f"Flame: {'🔥 YES' if flame=='0' else 'No'} | "
                    f"Gas: {gas} | "
                    f"Distance: {dist} cm"
                )

                self.sensor_text.set(text)

            except:
                pass

    # -------- CAMERA --------
    def update_camera(self):
        if not self.cap.isOpened():
            self.camera_label.config(text="❌ Camera not connected")
            self.root.after(1000, self.update_camera)
            return

        ret, frame = self.cap.read()

        if ret:
            frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            img = Image.fromarray(frame)
            img = img.resize((800, 500))
            imgtk = ImageTk.PhotoImage(img)

            self.camera_label.imgtk = imgtk
            self.camera_label.configure(image=imgtk, text="")
        else:
            # Don't spam errors
            pass

        self.root.after(30, self.update_camera)

    # -------- KEYBOARD --------
    def bind_keys(self):
        self.root.bind("<KeyPress-w>", lambda e: self.send('f'))
        self.root.bind("<KeyPress-s>", lambda e: self.send('b'))
        self.root.bind("<KeyPress-a>", lambda e: self.send('l'))
        self.root.bind("<KeyPress-d>", lambda e: self.send('r'))

        self.root.bind("<KeyRelease-w>", lambda e: self.send('s'))
        self.root.bind("<KeyRelease-s>", lambda e: self.send('s'))
        self.root.bind("<KeyRelease-a>", lambda e: self.send('s'))
        self.root.bind("<KeyRelease-d>", lambda e: self.send('s'))

# -------- RUN --------
root = tk.Tk()
app = RobotApp(root)
root.mainloop()