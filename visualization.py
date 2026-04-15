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
                        while ser.in_waiting:
                            line = ser.readline().decode('utf-8', errors='ignore').strip()
                            
                            if line and "DAFI_T:" in line:
                                # Parse using regex to handle negative numbers:
                                # Format: DAFI_T:{temp}_X:{x}_Y:{y}_Z:{z}
                                match = re.search(r'DAFI_T:(-?\d+)_X:(-?\d+)_Y:(-?\d+)_Z:(-?\d+)', line)
                                
                                if match:
                                    try:
                                        temp, x, y, z = map(int, match.groups())
                                        
                                        with serial_lock:
                                            data_buffer['timestamp'].append(datetime.now())
                                            data_buffer['x_axis'].append(x)
                                            data_buffer['y_axis'].append(y)
                                            data_buffer['z_axis'].append(z)
                                            data_buffer['temperature'].append(temp)
                                            data_ready = True
                                    except Exception as e:
                                        print(f"[Error] Parse error: {e}", flush=True)
                        
                        time.sleep(0.005)
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


def update_plot(frame, axes, lines, texts):
    """Update plot with latest data"""
    
    with serial_lock:
        if not data_buffer['timestamp']:
            return lines + texts
        
        timestamps = list(data_buffer['timestamp'])
        x_data = list(data_buffer['x_axis'])
        y_data = list(data_buffer['y_axis'])
        z_data = list(data_buffer['z_axis'])
        temp_data = list(data_buffer['temperature'])
    
    # Time axis (in seconds from start)
    if len(timestamps) > 1:
        time_axis = [(t - timestamps[0]).total_seconds() for t in timestamps]
    else:
        time_axis = [0] * len(timestamps)
    
    if not time_axis:
        return lines + texts

    # Update data on lines
    lines[0].set_data(time_axis, x_data)
    lines[1].set_data(time_axis, y_data)
    lines[2].set_data(time_axis, z_data)
    lines[3].set_data(time_axis, temp_data)
    
    # Adjust X limits for all axes to slide with the data
    min_t = time_axis[0]
    max_t = time_axis[-1] if time_axis[-1] > time_axis[0] else time_axis[0] + 1
    
    # Create padding for the text
    padding = (max_t - min_t) * 0.02
    
    axes[0][0].set_xlim(min_t, max_t + padding)
    axes[1][0].set_xlim(min_t, max_t + padding)
    axes[2][0].set_xlim(min_t, max_t + padding)
    axes[3][0].set_xlim(min_t, max_t + padding)
    
    # Update position and content of trailing text labels
    latest_t = time_axis[-1]
    
    texts[0].set_position((latest_t, x_data[-1]))
    texts[0].set_text(f' {x_data[-1]}')
    
    texts[1].set_position((latest_t, y_data[-1]))
    texts[1].set_text(f' {y_data[-1]}')
    
    texts[2].set_position((latest_t, z_data[-1]))
    texts[2].set_text(f' {z_data[-1]}')
    
    texts[3].set_position((latest_t, temp_data[-1]))
    texts[3].set_text(f' {temp_data[-1]}°C')
            
    return lines + texts


def setup_single_ax(ax, title, ylabel, ylim, line_color, text_color, xlabel=None):
    """Utility function to setup static properties for a single plot axis"""
    ax.set_title(title, fontsize=12, fontweight='bold')
    if xlabel:
        ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.grid(True, alpha=0.3)
    ax.set_ylim(*ylim)
    line, = ax.plot([], [], line_color, linewidth=2)
    text = ax.text(0, 0, '', color=text_color, fontsize=10, fontweight='bold', verticalalignment='center')
    return line, text


def setup_plot_axes(fig):
    """Set up the grid and all axes properties for the main figure."""
    gs = GridSpec(4, 1, figure=fig, hspace=0.3, wspace=0.4)
    axes = [[fig.add_subplot(gs[0, 0])],
            [fig.add_subplot(gs[1, 0])],
            [fig.add_subplot(gs[2, 0])],
            [fig.add_subplot(gs[3, 0])]]
    
    # Setup axes static properties
    line_x, text_x = setup_single_ax(
        axes[0][0], 'X Axis Acceleration', 'Value', (-1024, 1024), 'b-', 'blue'
    )
    line_y, text_y = setup_single_ax(
        axes[1][0], 'Y Axis Acceleration', 'Value', (-1024, 1024), 'g-', 'green'
    )
    line_z, text_z = setup_single_ax(
        axes[2][0], 'Z Axis Acceleration', 'Value', (-1024, 1024), 'r-', 'red'
    )
    line_temp, text_temp = setup_single_ax(
        axes[3][0], 'Temperature', '°C', (-40, 85), 'orange', 'darkorange', xlabel='Time (s)'
    )

    lines = [line_x, line_y, line_z, line_temp]
    texts = [text_x, text_y, text_z, text_temp]
    
    return axes, lines, texts


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
        
        axes, lines, texts = setup_plot_axes(fig)
        
        # Create animation
        print("[Main] Creating animation", flush=True)
        ani = animation.FuncAnimation(
            fig, 
            update_plot, 
            fargs=(axes, lines, texts),
            interval=50,  # Update every 50ms for smoother view
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
