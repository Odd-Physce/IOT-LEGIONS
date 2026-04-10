import requests

ESP32_IP = "http://10.201.220.153"

def send(cmd):
    try:
        requests.get(f"{ESP32_IP}/{cmd}", timeout=0.05)
    except:
        pass  # ignore errors completely

while True:
    cmd = input("f/b/l/r/s: ")

    if cmd == 'f':
        send("forward")
    elif cmd == 'b':
        send("back")
    elif cmd == 'l':
        send("left")
    elif cmd == 'r':
        send("right")
    elif cmd == 's':
        send("stop")