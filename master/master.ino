import serial
import csv
import time
import sys

COM_PORT = "COM3"  # ⚠️ CHANGE THIS to your ESP32 port
BAUD_RATE = 115200
DURATION = 30  # seconds

def capture_data(label, filename):
    """Capture sensor data from ESP32"""
    
    try:
        # Connect to ESP32
        ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
        print(f"✅ Connected to {COM_PORT}")
        time.sleep(2)
        
        # Create CSV
        with open(filename, 'w', newline='') as csvfile:
            fieldnames = ['timestamp', 'senderID', 'angleX', 'angleY']
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()
            
            print(f"\n📊 Collecting {label.upper()} data for {DURATION} seconds...")
            print("-" * 60)
            
            start_time = time.time()
            sample_count = 0
            
            while time.time() - start_time < DURATION:
                if ser.in_waiting:
                    try:
                        line = ser.readline().decode('utf-8', errors='ignore').strip()
                        
                        if line and ',' in line:
                            print(f"[{sample_count}] {line}")
                            
                            # Parse: senderID,angleX,angleY
                            parts = line.split(',')
                            if len(parts) == 3:
                                sender_id, angle_x, angle_y = parts
                                
                                # Write to CSV
                                writer.writerow({
                                    'timestamp': int((time.time() - start_time) * 1000),
                                    'senderID': sender_id,
                                    'angleX': angle_x,
                                    'angleY': angle_y
                                })
                                sample_count += 1
                    except Exception as e:
                        print(f"Parse error: {e}")
            
            ser.close()
            print("-" * 60)
            print(f"✅ {label.upper()} data saved: {filename} ({sample_count} samples)\n")
            
    except serial.SerialException as e:
        print(f"❌ Serial Error: {e}")
        print(f"❌ Available ports: Check Device Manager or use: python -m serial.tools.list_ports")
        sys.exit(1)

def main():
    print("=" * 60)
    print("SHM DATA COLLECTION TOOL")
    print("=" * 60)
    
    # Collect normal data
    print("\n🟢 STEP 1: Collecting NORMAL data")
    print("Keep sensors still (no vibrations)")
    input("Press Enter to start...")
    capture_data("normal", "normal_data.csv")
    
    # Wait between collections
    print("⏳ Wait 5 seconds before abnormal data collection...")
    for i in range(5, 0, -1):
        print(f"   {i}...", end='\r')
        time.sleep(1)
    
    # Collect abnormal data
    print("\n🔴 STEP 2: Collecting ABNORMAL data")
    print("Tap/shake sensors to create vibrations")
    input("Press Enter to start...")
    capture_data("abnormal", "abnormal_data.csv")
    
    print("\n" + "=" * 60)
    print("✅ DATA COLLECTION COMPLETE!")
    print("=" * 60)
    print("Files created:")
    print("  • normal_data.csv")
    print("  • abnormal_data.csv")
    print("\nNext: Upload to EdgeImpulse")
    print("=" * 60)

if __name__ == "__main__":
    main()