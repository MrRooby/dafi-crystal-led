#!/usr/bin/env python3
"""
DAFI Device Visualization - Live Graph of Sensor Data
Connects to DAFI device via UART and plots X, Y, Z acceleration and temperature
"""

import serial
import time
from collections import deque
from datetime import datetime
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.gridspec import GridSpec
import threading
import sys
import re

# Configuration
BAUD_RATE = 115200
TIMEOUT = 1
MAX_BUFFER_SIZE = 100  # Number of data points to display
DEVICE_IDENTIFIER = "DAFI"

# Storage for sensor data
data_buffer = {
    'timestamp': deque(maxlen=MAX_BUFFER_SIZE),
    'x_axis': deque(maxlen=MAX_BUFFER_SIZE),
    'y_axis': deque(maxlen=MAX_BUFFER_SIZE),
    'z_axis': deque(maxlen=MAX_BUFFER_SIZE),
    'temperature': deque(maxlen=MAX_BUFFER_SIZE),
}

serial_lock = threading.Lock()
ser = None
data_ready = False
reconnect_flag = False


def find_device():
    """Scan /dev/ttyACM0 to /dev/ttyACM5 for DAFI device"""
    print("Scanning for DAFI device...")
    for port_num in range(6):
        port = f"/dev/ttyACM{port_num}"
        try:
            s = serial.Serial(port, BAUD_RATE, timeout=TIMEOUT)
            print(f"  Trying {port}...", end=" ", flush=True)
            
            # Read a few lines to detect DAFI string
            time.sleep(0.5)  # Wait for device to respond
            start_time = time.time()
            
            while time.time() - start_time < 2:  # Try for 2 seconds
                if s.in_waiting:
                    line = s.readline().decode('utf-8', errors='ignore').strip()
                    if "DAFI" in line or "DAFi" in line:
                        print(f"✓ Found DAFI device on {port}")
                        return s
            
            s.close()
            print("✗")
        except (serial.SerialException, FileNotFoundError):
            print(f"  {port} not available")
            continue
    
    raise RuntimeError("DAFI device not found on any port (ttyACM0-5)")


def parse_sensor_data(line):
    """
    Parse a line of sensor data.
    Expected format from main.c:
    DAFI:T-25:X-123-Y:456-Z:789
    """
    try:
        if "DAFI:" not in line:
            return None
        
        # Parse format: DAFI:T-{temp}:X-{x}-Y:{y}-Z:{z}
        parts = line.split(':')
        if len(parts) < 4:
            return None
        
        # Extract temperature (T-value)
        if 'T-' in parts[0]:
            temp_str = parts[0].split('T-')[1]
            temp = int(temp_str)
            return ('temp', temp)
        
        # Extract X axis (X-value)
        if 'X-' in parts[1]:
            x_str = parts[1].split('X-')[1]
            x = int(x_str)
            return ('x', x)
        
        # Extract Y axis (Y:value)
        if 'Y:' in parts[2]:
            y_str = parts[2].split('Y:')[1]
            y = int(y_str)
            return ('y', y)
        
        # Extract Z axis (Z:value)
        if 'Z:' in parts[3]:
            z_str = parts[3].split('Z:')[1]
            z = int(z_str)
            return ('z', z)
    
    except (ValueError, IndexError):
        pass
    
    return None


def read_serial_data():
    """Read data from serial port in background thread"""
    global ser, data_ready, reconnect_flag
    
    print("[Thread] Serial reader started", flush=True)
    
    while True:
        try:
            if ser and ser.is_open:
                try:
                    if ser.in_waiting:
                        line = ser.readline().decode('utf-8', errors='ignore').strip()
                        print(f"[Serial RX] {repr(line)}", flush=True)
                        
                        if line and "DAFI_T:" in line:
                            # Parse using regex to handle negative numbers:
                            # Format: DAFI_T:{temp}_X:{x}_Y:{y}_Z:{z}
                            match = re.search(r'DAFI_T:(-?\d+)_X:(-?\d+)_Y:(-?\d+)_Z:(-?\d+)', line)
                            
                            if match:
                                try:
                                    temp, x, y, z = map(int, match.groups())
                                    print(f"[Parsed] T={temp}°C, X={x}, Y={y}, Z={z}", flush=True)
                                    
                                    with serial_lock:
                                        data_buffer['timestamp'].append(datetime.now())
                                        data_buffer['x_axis'].append(x)
                                        data_buffer['y_axis'].append(y)
                                        data_buffer['z_axis'].append(z)
                                        data_buffer['temperature'].append(temp)
                                        data_ready = True
                                        print(f"[Buffer] Size: {len(data_buffer['timestamp'])}", flush=True)
                                except Exception as e:
                                    print(f"[Error] Parse error: {e}", flush=True)
                            else:
                                print(f"[Warn] No regex match: {repr(line)}", flush=True)
                        
                        time.sleep(0.01)
                    else:
                        time.sleep(0.1)
                
                except (serial.SerialException, OSError):
                    # Port was closed or stopped responding
                    print("[Error] Port disconnected", flush=True)
                    try:
                        ser.close()
                    except:
                        pass
                    ser = None
                    reconnect_flag = True
                    time.sleep(1)
            else:
                time.sleep(0.5)
        
        except Exception as e:
            print(f"[Error] {e}", flush=True)
            time.sleep(1)


def update_plot(frame, axes):
    """Update plot with latest data"""
    
    with serial_lock:
        if not data_buffer['timestamp']:
            return
        
        timestamps = list(data_buffer['timestamp'])
        x_data = list(data_buffer['x_axis'])
        y_data = list(data_buffer['y_axis'])
        z_data = list(data_buffer['z_axis'])
        temp_data = list(data_buffer['temperature'])
    
    # Clear all axes
    for row in axes:
        for ax in row:
            ax.clear()
    
    # Time axis (in seconds from start)
    if len(timestamps) > 1:
        time_axis = [(t - timestamps[0]).total_seconds() for t in timestamps]
    else:
        time_axis = [0]
    
    # X Axis plot
    axes[0][0].plot(time_axis, x_data, 'b-', linewidth=2, marker='o', markersize=4)
    axes[0][0].set_title('X Axis Acceleration', fontsize=12, fontweight='bold')
    axes[0][0].set_ylabel('Value')
    axes[0][0].grid(True, alpha=0.3)
    
    # Y Axis plot
    axes[1][0].plot(time_axis, y_data, 'g-', linewidth=2, marker='o', markersize=4)
    axes[1][0].set_title('Y Axis Acceleration', fontsize=12, fontweight='bold')
    axes[1][0].set_ylabel('Value')
    axes[1][0].grid(True, alpha=0.3)
    
    # Z Axis plot
    axes[2][0].plot(time_axis, z_data, 'r-', linewidth=2, marker='o', markersize=4)
    axes[2][0].set_title('Z Axis Acceleration', fontsize=12, fontweight='bold')
    axes[2][0].set_ylabel('Value')
    axes[2][0].grid(True, alpha=0.3)
    
    # Temperature plot
    axes[3][0].plot(time_axis, temp_data, 'orange', linewidth=2, marker='o', markersize=4)
    axes[3][0].set_title('Temperature', fontsize=12, fontweight='bold')
    axes[3][0].set_xlabel('Time (s)')
    axes[3][0].set_ylabel('°C')
    axes[3][0].set_ylim(-40, 85)  # Fixed range -40 to 85°C
    axes[3][0].grid(True, alpha=0.3)
    
    # Set y-axis limits with some padding
    # XYZ graphs: fixed range ±1024
    axes[0][0].set_ylim(-1024, 1024)
    axes[1][0].set_ylim(-1024, 1024)
    axes[2][0].set_ylim(-1024, 1024)


def main():
    global ser, reconnect_flag
    
    try:
        print("[Main] Starting DAFI visualization", flush=True)
        
        # Start serial reading thread first
        print("[Main] Starting serial thread", flush=True)
        serial_thread = threading.Thread(target=read_serial_data, daemon=True)
        serial_thread.start()
        
        # Find and connect to device
        print("[Main] Finding device", flush=True)
        ser = find_device()
        print(f"[Main] Connected to {ser.port}", flush=True)
        
        # Setup matplotlib figure
        print("[Main] Setting up figure", flush=True)
        fig = plt.figure(figsize=(12, 12))
        fig.suptitle('DAFI Sensor Data - Live Graph', fontsize=14, fontweight='bold')
        
        gs = GridSpec(4, 1, figure=fig, hspace=0.3, wspace=0.4)
        axes = [[fig.add_subplot(gs[0, 0])],
                [fig.add_subplot(gs[1, 0])],
                [fig.add_subplot(gs[2, 0])],
                [fig.add_subplot(gs[3, 0])]]
        
        # Create animation
        print("[Main] Creating animation", flush=True)
        ani = animation.FuncAnimation(
            fig, 
            update_plot, 
            fargs=(axes,),
            interval=200,  # Update every 200ms
            blit=False,
            cache_frame_data=False
        )
        
        print("[Main] Starting live plot... (Close window to exit)", flush=True)
        plt.show()
    
    except KeyboardInterrupt:
        print("\n[Main] Exiting...", flush=True)
    
    except Exception as e:
        print(f"[Main] Error: {e}", flush=True)
        import traceback
        traceback.print_exc()
        sys.exit(1)
    
    finally:
        if ser and ser.is_open:
            ser.close()
            print("[Main] Serial port closed", flush=True)


if __name__ == '__main__':
    main()
