import requests

ESP32_IP = "http://10.201.220.153"

def send(command):
    try:
        url = f"{ESP32_IP}/{command}"
        response = requests.get(url)
        print(response.text)
    except:
        print("Error connecting")

while True:
    cmd = input("Enter command (f/b/l/r/s): ")

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
    else:
        print("Invalid")