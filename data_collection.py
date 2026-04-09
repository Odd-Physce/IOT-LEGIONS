import serial
import csv
import time
from datetime import datetime

# Configuration
COM_PORT = "COM11"  # Change to your ESP32 port (COM3, COM4, etc.)
BAUD_RATE = 115200
FILENAME = "sensor_data.csv"

def capture_serial_data():
    try:
        # Open serial connection
        ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
        print(f"Connected to {COM_PORT}")
        time.sleep(2)  # Wait for ESP32 to initialize
        
        # Create CSV file
        with open(FILENAME, 'w', newline='') as csvfile:
            fieldnames = ['timestamp', 'senderID', 'angleX', 'angleY', 'label']
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()
            
            print("\nSerial Monitor (Press Ctrl+C to stop):")
            print("-" * 50)
            
            data_count = 0
            label = input("Enter label (normal/abnormal): ").strip()
            
            ser.write(b"start\n")  # Send start command to ESP32
            
            while True:
                if ser.in_waiting:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    
                    if line:
                        print(line)
                        
                        # Parse CSV format: timestamp,senderID,angleX,angleY
                        try:
                            parts = line.split(',')
                            if len(parts) == 4:
                                timestamp, sender_id, angle_x, angle_y = parts
                                
                                # Write to CSV with label
                                writer.writerow({
                                    'timestamp': timestamp,
                                    'senderID': sender_id,
                                    'angleX': angle_x,
                                    'angleY': angle_y,
                                    'label': label
                                })
                                data_count += 1
                        except:
                            pass
        
        ser.close()
        print(f"\n✅ Data saved to {FILENAME} ({data_count} samples)")
        
    except Exception as e:
        print(f"❌ Error: {e}")

if __name__ == "__main__":
    capture_serial_data()