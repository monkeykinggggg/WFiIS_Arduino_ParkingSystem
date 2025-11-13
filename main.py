import tkinter as tk
from tkinter import ttk, messagebox
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.pyplot as plt
from collections import deque
from datetime import datetime
import pandas as pd
import matplotlib.dates as mdates
import time
import csv
import random
import sys
import threading


ser = None

import serial
SERIAL_PORT = "/dev/cu.usbmodem11101"  # COM05
BAUD_RATE = 9600
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)
except Exception as e:
    print("Nie udało się otworzyć portu szeregowego:", e)   # random data will be used
    ser = None

LOG_FILE = "parking_log.csv"

running = False
reading_thread = None
data_lock = threading.Lock()
data_points = deque()
window_seconds = 30

last_collision = 0

with open(LOG_FILE, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["timestamp", "rear_center", "rear_left", "rear_right", "collision"])


def reader_thread_func():
    global running, data_points, last_collision
    while running:
        try:
            if ser:
                raw = ser.readline().decode("utf-8").strip()
            else:
                rear_center = random.randint(5, 40)
                rear_left = random.randint(5, 40)
                rear_right = random.randint(5, 40)
                collision = 1 if random.random() < 0.05 else 0
                raw = f"{rear_center},{rear_left},{rear_right},{collision}"
                time.sleep(0.1)

            if not raw:
                continue

            parts = raw.split(",")
            # print("Odebrano:", parts)
            if len(parts) < 4:
                continue

            rear_center, rear_left, rear_right, collision = parts
            rear_center = int(rear_center); rear_left = int(rear_left); rear_right = int(rear_right)
            collision = int(collision)
            timestamp = datetime.now()
            print(f"read: {parts}")

            with data_lock:
                data_points.append((timestamp, rear_center, rear_left, rear_right, collision))
                while data_points and (timestamp - data_points[0][0]).total_seconds() > window_seconds:
                    data_points.popleft()
                last_collision = collision
                
                with open(LOG_FILE, "a", newline="") as f:
                    writer = csv.writer(f)
                    writer.writerow([timestamp.strftime("%H:%M:%S"), rear_center, rear_left, rear_right, collision])

        except Exception as e:
            print("Błąd w wątku czytającym:", e, file=sys.stderr)
            time.sleep(0.2)


def start_reading():
    global running, reading_thread
    if running:
        return
    running = True
    reading_thread = threading.Thread(target=reader_thread_func, daemon=True)
    reading_thread.start()
    root.after(100, update_gui)


def stop_reading():
    global running
    running = False


def send_to_arduino():
    mode = mode_var.get()
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
            root.after(100, update_gui)
            return

        snapshot = list(data_points)

    times = [t for t, *_ in snapshot] # time and everything else
    centers = [c for _, c, _, _, _ in snapshot]
    lefts = [l for _, _, l, _, _  in snapshot]
    rights = [r for _, _, _, r, _ in snapshot]

    ax_plot.clear()
    ax_car.clear()

    ax_plot.plot(times, lefts, label="Rear Left", linestyle='-', marker=None)
    ax_plot.plot(times, centers, label="Rear Center", linestyle='-', marker=None)
    ax_plot.plot(times, rights, label="Rear Right", linestyle='-', marker=None)
    ax_plot.set_ylim(0, 60)
    ax_plot.set_ylabel("Distance (cm)")
    ax_plot.set_xlabel("Time for last {} s".format(window_seconds))
    ax_plot.set_title("Rear Sensor Distance (Real-Time)")
    ax_plot.legend(loc="upper right")
    ax_plot.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M:%S"))
    ax_plot.set_xlim(datetime.now() - pd.Timedelta(seconds=window_seconds), datetime.now())
    plt.setp(ax_plot.xaxis.get_majorticklabels(), rotation=20, ha="left", fontsize=8)

# 0, 0 is at the start of the car rear
    ax_car.set_xlim(-100, 100)
    ax_car.set_ylim(-160, 160)
    ax_car.set_aspect("equal")
    ax_car.axis("off")
    ax_car.set_title("Car Sensors")

    car = plt.Rectangle((-30, -60), 60, 120, fill=True, color="gray", alpha=0.4)
    ax_car.add_patch(car)

    front_color = "red" if last_collision else "darkred"
    front_markersize = 10 if last_collision else 6
    ax_car.plot([-30, 30], [60, 60], 'o', color=front_color, markersize=front_markersize)

    ax_car.plot([-30, 0, 30], [-60, -60, -60], 'o', color="blue")

    # lines
    if snapshot:
        _, last_center, last_left, last_right, _ = snapshot[-1]
        print(f"rysunek: {snapshot[-1]}")
        ax_car.plot([-30, -30], [-60, -60 - last_left], 'b--')
        ax_car.plot([0, 0], [-60, -60 - last_center], 'b--')
        ax_car.plot([30, 30], [-60, -60 - last_right], 'b--')

        avg_rear = (last_left + last_center + last_right) // 3
        ax_car.text(0, -80 - avg_rear, f"{avg_rear} cm", color='blue', ha='center')

    rear_text = "REAR COLLISION!" if last_collision else ""
    if last_collision:
        ax_car.text(0, 90, rear_text, color='red', ha='center', fontsize=12, fontweight='bold')

    canvas.draw_idle()

    if running:
        root.after(100, update_gui)


root = tk.Tk()
root.title("Arduino Parking Assistant")

frame = tk.Frame(root)
frame.pack(side=tk.TOP, fill=tk.X, padx=6, pady=6)

tk.Label(frame, text="Collision Sound:").grid(row=0, column=0, padx=4)
mode_var = tk.StringVar(value="MARIO")
ttk.OptionMenu(frame, mode_var, "MARIO", "MARIO", "GAMEOVER", "PACMAN", "SQUIDGAME", "TOKYO_DRIFT").grid(row=0, column=1)

ttk.Button(frame, text="Send", command=send_to_arduino).grid(row=0, column=4, padx=8)
ttk.Button(frame, text="Start", command=start_reading).grid(row=0, column=5, padx=4)
ttk.Button(frame, text="Stop", command=stop_reading).grid(row=0, column=6, padx=4)

fig, (ax_plot, ax_car) = plt.subplots(1, 2, figsize=(9, 4))
canvas = FigureCanvasTkAgg(fig, master=root)
canvas_widget = canvas.get_tk_widget()
canvas_widget.pack(side=tk.BOTTOM, fill=tk.BOTH, expand=True)

canvas.draw_idle()

def on_closing():
    global running
    if running:
        if not messagebox.askokcancel("Zamykanie", "Logowanie jest włączone. Zatrzymać i zamknąć?"):
            return
    running = False
    try:
        if ser:
            ser.close()
    except Exception:
        pass
    root.destroy()

root.protocol("WM_DELETE_WINDOW", on_closing)

start_reading()
root.mainloop()