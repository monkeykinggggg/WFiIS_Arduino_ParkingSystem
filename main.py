import tkinter as tk
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.patches import Polygon
import matplotlib.dates as mdates

from collections import deque
from datetime import datetime
from pandas import Timedelta
import time
import random
import sys
import threading
import serial

ser = None
SERIAL_PORT = "COM5"  # /dev/cu.usbmodem11101
BAUD_RATE = 9600

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)
except Exception as e:
    print("Nie udało się otworzyć portu szeregowego:", e)
    ser = None

LOG_FILE = "parking_log.csv"

with open(LOG_FILE, "w", newline="") as f:
    f.write("timestamp, rear_left, rear_center, rear_right, collision\n")

reading_thread = None
data_lock = threading.Lock()
data_points = deque()
window_seconds = 30
last_collision = 0


def reader_thread_func():
    global data_points, last_collision, ser
    while True:
        try:
            if ser:
                raw = ser.readline().decode("utf-8").strip()
            else:
                rear_left = random.randint(30, 45)
                rear_center = random.randint(80, 100)
                rear_right = random.randint(0, 30)
                collision = 0
                raw = f"{rear_center},{rear_left},{rear_right},{collision}"
                time.sleep(0.1)

            if not raw:
                continue

            parts = raw.split(",")
            if len(parts) < 4:
                continue

            rear_left, rear_center, rear_right, collision = parts
            rear_center = int(rear_center)
            rear_left = int(rear_left)
            rear_right = int(rear_right)
            collision = int(collision)
            timestamp = datetime.now()

            with data_lock:
                data_points.append((timestamp, rear_left, rear_center, rear_right, collision))
                while data_points and (timestamp - data_points[0][0]).total_seconds() > window_seconds:
                    data_points.popleft()
                last_collision = collision

                with open(LOG_FILE, "a", newline="") as f:
                    f.write(
                        f'{timestamp.strftime("%H:%M:%S"):6}, {rear_left:10}, {rear_center:10}, {rear_right:10}, {collision:10}\n')

        except Exception as e:
            print("Błąd w wątku czytającym:", e, file=sys.stderr)
            time.sleep(0.2)


def start_reading():
    global reading_thread
    try:
        if ser:
            ser.write(b"START\n")
    except Exception as e:
        print("Błąd wysyłania do Arduino:", e)
    if reading_thread is None:
        reading_thread = threading.Thread(target=reader_thread_func, daemon=True)
        reading_thread.start()
        window.after(100, update_gui)


def stop_reading():
    try:
        if ser:
            ser.write(b"STOP\n")
    except Exception as e:
        print("Błąd wysyłania do Arduino:", e)


def measure_once():
    try:
        if ser:
            ser.write(b"MEASURE\n")
        print("Wysłano:", "MEASURE")
    except Exception as e:
        print("Błąd wysyłania do Arduino:", e)


def send_to_arduino():
    mode = dflt_sound.get()
    msg = f"{mode}\n"
    try:
        if ser:
            ser.write(msg.encode())
        print("Wysłano:", msg.strip())
    except Exception as e:
        print("Błąd wysyłania do Arduino:", e)


def update_gui():
    with data_lock:
        if not data_points:
            pass

        snapshot = list(data_points)

    times = [t for t, *_ in snapshot]
    centers = [c for _, _, c, _, _ in snapshot]
    lefts = [l for _, l, _, _, _ in snapshot]
    rights = [r for _, _, _, r, _ in snapshot]

    ax_plot.clear()
    ax_car.clear()

    ax_plot.grid(True)
    ax_plot.plot(times, lefts, label="Rear Left", linewidth=1)
    ax_plot.plot(times, centers, label="Rear Center", linewidth=1)
    ax_plot.plot(times, rights, label="Rear Right", linewidth=1)
    ax_plot.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M:%S"))
    ax_plot.set_xlim(datetime.now() - Timedelta(seconds=window_seconds), datetime.now())
    ax_plot.tick_params(axis='x', labelsize=5, rotation=60)
    ax_plot.tick_params(axis='y', labelsize=8)
    ax_plot.set_ylim(0, 60)
    ax_plot.set_ylabel("Distance (cm)", fontsize=8)
    ax_plot.set_xlabel("Time (last {} s)".format(window_seconds), fontsize=8)
    ax_plot.set_title("Rear Sensor Distance (Real-Time)", fontsize=8)
    ax_plot.legend(loc="upper right", fontsize=8)

    ax_car.set_xlim(-100, 100)
    ax_car.set_ylim(-160, 160)
    ax_car.set_aspect("equal")
    ax_car.tick_params(left=False, bottom=False, labelleft=False, labelbottom=False)
    ax_car.set_title("Car Sensors Detection", fontsize=8)

    car = plt.Rectangle((-30, -60), 60, 120, fill=True, color="gray", alpha=0.4)
    ax_car.add_patch(car)

    bumper_color = "red" if last_collision else "darkred"
    bumper_width = 5 if last_collision else 2
    ax_car.plot([-30, 30], [-60, -60], color=bumper_color, linewidth=bumper_width)
    ax_car.plot([-30, 0, 30], [-60, -60, -60], 'o', color="blue")

    def draw_sensor_cone(origin, heading, distance, text_align):
        ox, oy = origin
        hx, hy = heading
        length = (hx ** 2 + hy ** 2) ** 0.5 or 1
        hx /= length
        hy /= length
        tip = (ox, oy)
        base_cx = ox + hx * distance
        base_cy = oy + hy * distance
        px, py = -hy, hx
        base_half_width = 20
        base_left = (base_cx + px * base_half_width, base_cy + py * base_half_width)
        base_right = (base_cx - px * base_half_width, base_cy - py * base_half_width)
        cone = Polygon([tip, base_left, base_right], closed=True, color='blue', alpha=0.25, edgecolor='blue', linewidth=1)
        ax_car.add_patch(cone)
        text_x = base_cx + hx * 5
        text_y = base_cy + hy * 5
        ax_car.text(text_x, text_y, f"{distance}", color='blue', fontsize=8, ha=text_align, va='center')

    if snapshot:
        _, last_left, last_center, last_right, _ = snapshot[-1]
        draw_sensor_cone((-30, -60), (-0.6, -1), last_left, 'right')
        draw_sensor_cone((0, -60), (0, -1), last_center, 'center')
        draw_sensor_cone((30, -60), (0.6, -1), last_right, 'left')

    rear_text = "REAR COLLISION!" if last_collision else ""
    if last_collision:
        ax_car.text(0, 90, rear_text, color='red', ha='center', fontsize=12, fontweight='bold')

    canvas.draw()

    window.after(100, update_gui)

# initializing main window
window = tk.Tk()
window.title("Arduino Parking Assistant")
window.maxsize(1200, 1000)

frame = tk.Frame(window)
frame.pack(side=tk.TOP, fill=tk.BOTH, padx=20, pady=10)

tk.Label(frame, text="Collision Sound:").grid(row=0, column=0)
dflt_sound = tk.StringVar(value="MARIO")  # automatically updates when the OptionMenu selection changes
option_sound_list = ["MARIO", "GAMEOVER", "PACMAN", "SQUIDGAME", "TOKYO_DRIFT"]
tk.OptionMenu(frame, dflt_sound, *option_sound_list).grid(row=0, column=1, pady=5)

tk.Button(frame, text="Measure", command=measure_once).grid(row=0, column=3, padx=4, pady=5)
tk.Button(frame, text="Send", command=send_to_arduino).grid(row=0, column=4, padx=4, pady=5)
tk.Button(frame, text="Start", command=start_reading).grid(row=0, column=5, padx=4, pady=5)
tk.Button(frame, text="Stop", command=stop_reading).grid(row=0, column=6, padx=4, pady=5)

fig, (ax_plot, ax_car) = plt.subplots(1, 2, figsize=(8, 4))
plt.tight_layout(pad=3.0)
canvas = FigureCanvasTkAgg(fig, master=window)
canvas.get_tk_widget().pack(side=tk.BOTTOM, expand=True, fill=tk.BOTH, padx=10, pady=10)
canvas.draw()

def on_closing():
    global ser, window
    try:
        if ser:
            ser.close()
    except Exception:
        pass
    window.destroy()

window.protocol("WM_DELETE_WINDOW", on_closing)

start_reading()
window.mainloop()