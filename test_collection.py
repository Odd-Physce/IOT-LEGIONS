import serial
import time

COM_PORT = "COM11"  # Change to your port
BAUD_RATE = 115200

try:
    ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
    print(f"Connected to {COM_PORT}")
    time.sleep(2)
    
    print("Reading data for 10 seconds...")
    start = time.time()
    
    while time.time() - start < 10:
        if ser.in_waiting:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            print(f"Received: {line}")
    
    ser.close()
    print("Done!")
    
except Exception as e:
    print(f"Error: {e}")