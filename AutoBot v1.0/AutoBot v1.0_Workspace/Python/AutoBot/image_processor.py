"""
autobot_gui_final_with_scale.py
AUTOBOT GUI with scalable font system (single FONT_SCALE knob).
Change FONT_SCALE near the top to compare sizes (1.1 / 1.2 / 1.35 / 1.5).
Includes fix: Tkinter StringVar created AFTER CTk root window.
"""

import threading
import queue
import time
import os
import json
import csv
import serial
import serial.tools.list_ports
import customtkinter as ctk
from tkinter import BOTH, LEFT, RIGHT, X, Y, TOP, BOTTOM
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from PIL import Image, ImageTk, ImageEnhance

import firebase_admin
from firebase_admin import credentials, db



# ---------------- CONFIG ----------------
DEFAULT_COM = "COM16"
DEFAULT_BAUD = 115200
CSV_LOG_PATH = r"C:\Users\Public\Autobot_Log.csv"
CSV_BATCH_INTERVAL = 1.0
GUI_POLL_MS = 50
MAX_GUI_QUEUE = 1200
MAX_LOG_QUEUE = 5000

os.makedirs(os.path.dirname(CSV_LOG_PATH), exist_ok=True)

# Queues and flags
gui_queue = queue.Queue(maxsize=MAX_GUI_QUEUE)
log_queue = queue.Queue(maxsize=MAX_LOG_QUEUE)

serial_stop_event = threading.Event()
serial_connected = threading.Event()
logging_enabled = threading.Event()
write_to_csv_flag = threading.Event()
serial_lock = threading.Lock()
ser = None


# ---------------- Helpers ----------------
def list_serial_ports():
    return [p.device for p in serial.tools.list_ports.comports()]


def pad_list(arr, n=3):
    if not isinstance(arr, list):
        return [0.0] * n
    out = arr[:n] + [0.0] * max(0, n - len(arr))
    res = []
    for x in out:
        try:
            res.append(float(x))
        except:
            res.append(0.0)
    return res


def safe_int(v, default=0):
    try:
        return int(v)
    except:
        try:
            return int(float(v))
        except:
            return default


def safe_float(v, default=0.0):
    try:
        return float(v)
    except:
        return default


def prepare_row_for_csv(data):
    ts = time.strftime("%Y-%m-%d %H:%M:%S")
    enc = data.get("enc", {}) or {}
    imu = data.get("imu", {}) or {}
    battery = data.get("battery", {}) or {}
    esp = data.get("esp", {}) or {}

    acc = pad_list(imu.get("acc", []), 3)
    gyro = pad_list(imu.get("gyro", []), 3)
    euler = pad_list(imu.get("euler", []), 3)

    pos_raw = esp.get("pos", [0, 0, 0])
    if not isinstance(pos_raw, list):
        pos_raw = [0, 0, 0]
    pos = (pos_raw[:3] + [0.0] * 3)[:3]
    pos = [safe_float(x, 0.0) for x in pos]

    row = [
        ts,
        safe_int(enc.get("L", 0)), safe_int(enc.get("R", 0)),
        safe_float(enc.get("left_deg", 0.0)), safe_float(enc.get("right_deg", 0.0)),
        acc[0], acc[1], acc[2],
        gyro[0], gyro[1], gyro[2],
        euler[0], euler[1], euler[2],
        safe_float(battery.get("voltage", 0.0)), safe_int(battery.get("percent", 0)),
        safe_int(esp.get("tag", 0)),
        safe_float(esp.get("yaw", 0.0)), safe_float(esp.get("pitch", 0.0)), safe_float(esp.get("roll", 0.0)),
        pos[0], pos[1], pos[2]
    ]
    return row

def firebase_listener_thread():
    ref = db.reference("/AUTOBOT/AUTOBOT")   # << FIXED PATH

    prev_pick = None
    prev_drop = None

    while True:
        try:
            data = ref.get() or {}

            new_pick = data.get("PickUpBlock", "-")
            new_drop = data.get("DropBlock", "-")

            # update GUI only if values changed
            if new_pick != prev_pick:
                pickup_val.configure(text=str(new_pick))
                prev_pick = new_pick

            if new_drop != prev_drop:
                drop_val.configure(text=str(new_drop))
                prev_drop = new_drop

        except Exception as e:
            print("Firebase error:", e)

        time.sleep(0.5)


# ---------------- GUI Construction ----------------
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")

app = ctk.CTk()
app.title("AUTOBOT - Live GUI (Final)")
app.geometry("1280x760")

# ------------- FIREBASE INIT --------------
cred = credentials.Certificate("autobot-20dfa-firebase-adminsdk-fbsvc-6972378650.json")

firebase_admin.initialize_app(cred, {
    "databaseURL": "https://autobot-20dfa-default-rtdb.firebaseio.com/"
})

# ---------------- FONT CONFIG (CONTROLLED GLOBAL SCALING) ----------------
# Change FONT_SCALE to try different presets:
# Option A (subtle): 1.1
# Option B (medium): 1.2
# Option C (large): 1.35
# Option D (huge): 1.5
# ---------------- FONT CONFIG ----------------
FONT_SCALE = 1.2

def fs(px):
    return max(1, int(px * FONT_SCALE))

# CUSTOM FONTS (use EXACT installed names)
FONT_HEADER = ("Ethnocentric", fs(50), "bold", "italic")
FONT_SUBHEADER = ("Orbitron", fs(16))               # Tagline font

# UI fonts
FONT_TITLE = ("Segoe UI", fs(14), "bold")
FONT_SUBTITLE = ("Segoe UI", fs(12), "bold")
FONT_TILE_TITLE = ("Segoe UI", fs(12), "bold")
FONT_TILE_VALUE_LARGE = ("Consolas", fs(36), "bold")
FONT_VALUE_LARGE = ("Consolas", fs(30), "bold")
FONT_VALUE_MED = ("Consolas", fs(28), "bold")
FONT_VALUE_MED_SMALL = ("Consolas", fs(22), "bold")
FONT_VALUE_SMALL = ("Consolas", fs(20), "bold")
FONT_SMALL = ("Consolas", fs(16))
FONT_BUTTON = ("Segoe UI", fs(12), "bold")
FONT_TAG_TITLE = ("Segoe UI", fs(20), "bold")
FONT_TAG_VALUE = ("Consolas", fs(50), "bold")


# ---------------- NOW WE CAN SAFELY CREATE TK VARIABLES (THE FIX) ----------------
yaw_scale_presets = ["-180 to +180", "-90 to +90", "-360 to +360", "Auto"]
yaw_scale_var = ctk.StringVar(value=yaw_scale_presets[0])
yaw_auto_scale = False
yaw_manual_min = -180.0
yaw_manual_max = 180.0
YAW_SCALE_MARGIN = 10.0


# ---------------- HEADER (FIXED, NO SHIFTING, REAL IMAGE BACKGROUND) ----------------
from tkinter import Canvas

HEADER_H = 110    # do NOT change, keeps GUI proportions identical

header_canvas = Canvas(app, width=1280, height=HEADER_H, highlightthickness=0, bg="black")
header_canvas.pack(fill=X, padx=8, pady=(8, 6))

try:
    img_path = r"C:\Users\kaver\OneDrive\Desktop\C_files\Python\AutoBot\AUTOBOT_GUI_BANNER.jpg"

    raw = Image.open(img_path)
    resized = raw.resize((1280, HEADER_H), Image.LANCZOS)

    enhancer = ImageEnhance.Brightness(resized)
    dark = enhancer.enhance(0.5)   # darker, almost black but visible
    tk_header = ImageTk.PhotoImage(dark)

    # create image and keep its item id
    header_bg_id = header_canvas.create_image(0, 0, anchor="nw", image=tk_header)
    header_canvas.image = tk_header  # keep reference


    # function to keep image centered in the canvas
    def center_header_image(event=None):
        canvas_w = header_canvas.winfo_width()
        header_canvas.coords(header_bg_id, canvas_w // 2, HEADER_H // 2)
        header_canvas.itemconfig(header_bg_id, anchor="center")


    # call once now and also whenever the canvas resizes
    center_header_image()
    header_canvas.bind("<Configure>", center_header_image)


except Exception as e:
    print("HEADER IMAGE ERROR:", e)

# AUTOBOT Title
header_canvas.create_text(
    925, 38,
    text="AUTOBOT",
    fill="white",
    font=FONT_HEADER
)

# Tagline
header_canvas.create_text(
    925, 78,
    text="AUTOMATING YOUR EVERYDAY TASK",
    fill="#cccccc",
    font=FONT_SUBHEADER
)



# Main split
main = ctk.CTkFrame(app)
main.pack(fill=BOTH, expand=True, padx=12, pady=8)

# LEFT panel
left_panel = ctk.CTkFrame(main)
left_panel.pack(side=LEFT, fill=BOTH, expand=True, padx=(0, 8))

# Graph frame
graph_frame = ctk.CTkFrame(left_panel, fg_color="#111111", corner_radius=8)
graph_frame.pack(fill=X, padx=6, pady=(6, 10))
graph_frame.pack_propagate(False)
graph_frame.configure(height=360)

ctk.CTkLabel(graph_frame, text="YAW LIVE GRAPH", font=FONT_TITLE).pack(anchor="w", padx=10, pady=(8, 0))

fig = Figure(figsize=(4, 4), dpi=100)
fig.patch.set_facecolor("#111111")
ax = fig.add_subplot(111)
ax.set_facecolor("black")
ax.grid(True, linestyle="--", linewidth=0.5, alpha=0.4)
ax.tick_params(colors="white")
ax.set_title("Yaw (recent)", color="white", fontsize=fs(10))
fig.subplots_adjust(left=0.08, right=0.98, top=0.92, bottom=0.12)

yaw_canvas = FigureCanvasTkAgg(fig, master=graph_frame)
yaw_canvas.get_tk_widget().pack(fill=BOTH, expand=True, padx=8, pady=(6, 10))

yaw_history = []
MAX_POINTS = 90


# ---------------- IMU grid ----------------
imu_grid = ctk.CTkFrame(left_panel)
imu_grid.pack(fill=BOTH, expand=False, padx=6, pady=(0, 8))
imu_grid.grid_propagate(False)
imu_grid.configure(height=360)

tiles = [
    "Accel X (g)", "Accel Y (g)", "Accel Z (g)",
    "Gyro X (°/s)", "Gyro Y (°/s)", "Gyro Z (°/s)",
    "Pitch", "Roll", "Yaw"
]

imu_tiles = {}

for i, title in enumerate(tiles):
    r = i // 3
    c = i % 3
    tile = ctk.CTkFrame(imu_grid, fg_color="#1a1a1a", corner_radius=6)
    tile.grid(row=r, column=c, sticky="nsew", padx=6, pady=6)
    tile.grid_propagate(False)  # << prevent tile resizing

    ctk.CTkLabel(tile, text=title, font=FONT_TILE_TITLE).pack(pady=(8, 4))
    # FIXED VALUE FRAME
    value_frame = ctk.CTkFrame(tile, fg_color="transparent")
    value_frame.pack(pady=(4, 10), fill="x")

    value_frame.configure(width=fs(120), height=fs(40))
    value_frame.pack_propagate(False)  # << prevents size changes

    val = ctk.CTkLabel(
        value_frame,
        text="0.00",
        font=FONT_TILE_VALUE_LARGE,
        text_color="#ffcc00",
        anchor="center"
    )
    val.pack(expand=True)

    imu_tiles[title] = val

for c in range(3):
    imu_grid.columnconfigure(c, weight=1)
for r in range(3):
    imu_grid.rowconfigure(r, weight=1)


# ---------------- Encoders ----------------
enc_row = ctk.CTkFrame(left_panel)
enc_row.pack(fill=X, padx=6, pady=(0, 8))

enc_left = ctk.CTkFrame(enc_row, fg_color="#1a1a1a", corner_radius=6)
enc_left.pack(side=LEFT, fill=BOTH, expand=True, padx=(0, 6))
ctk.CTkLabel(enc_left, text="Left Encoder", font=FONT_SUBTITLE).pack(pady=(8, 4))
left_enc_lbl = ctk.CTkLabel(enc_left, text="0", font=FONT_VALUE_MED_SMALL,
                            width=fs(110), anchor="center")
left_enc_lbl.pack(pady=(4, 6))
left_deg_lbl = ctk.CTkLabel(enc_left, text="[0.00°]", font=FONT_SMALL,
                            width=fs(110), anchor="center")
left_deg_lbl.pack(pady=(0, 12))
enc_left.grid_propagate(False)

enc_right = ctk.CTkFrame(enc_row, fg_color="#1a1a1a", corner_radius=6)
enc_right.pack(side=RIGHT, fill=BOTH, expand=True, padx=(6, 0))
ctk.CTkLabel(enc_right, text="Right Encoder", font=FONT_SUBTITLE).pack(pady=(8, 4))
right_enc_lbl = ctk.CTkLabel(enc_right, text="0", font=FONT_VALUE_MED_SMALL,
                             width=fs(110), anchor="center")
right_enc_lbl.pack(pady=(4, 6))
right_deg_lbl = ctk.CTkLabel(enc_right, text="[0.00°]", font=FONT_SMALL,
                             width=fs(110), anchor="center")
right_deg_lbl.pack(pady=(0, 12))
enc_right.grid_propagate(False)


# ---------------- RIGHT panel ----------------
right_panel = ctk.CTkFrame(main, width=380)
right_panel.pack(side=RIGHT, fill=Y, padx=(8, 0))


# COM connection
com_frame = ctk.CTkFrame(right_panel, fg_color="#121212", corner_radius=8)
com_frame.pack(fill=X, padx=8, pady=(6, 8))
ctk.CTkLabel(com_frame, text="COM / CONNECTION", font=FONT_SUBTITLE).pack(anchor="w", padx=10, pady=(8, 4))

ports = list_serial_ports()
if not ports:
    ports = [DEFAULT_COM]

com_var = ctk.StringVar(value=ports[0])
baud_var = ctk.IntVar(value=DEFAULT_BAUD)

ctk.CTkOptionMenu(com_frame, values=ports, variable=com_var, width=120).pack(side=LEFT, padx=(10, 6))
ctk.CTkEntry(com_frame, textvariable=baud_var, width=90).pack(side=LEFT, padx=(0, 6))
connect_btn = ctk.CTkButton(
    com_frame,
    text="Connect",
    width=90,
    font=FONT_BUTTON,
    fg_color="transparent",
    border_color="#22c55e",   # green border
    border_width=2,
    hover_color=("gray25")
)

disconnect_btn = ctk.CTkButton(
    com_frame,
    text="Disconnect",
    width=90,
    font=FONT_BUTTON,
    fg_color="transparent",
    border_color="#ef4444",   # red border
    border_width=2,
    hover_color=("gray25"),
    state="disabled"
)


connect_btn.pack(side=LEFT, padx=6)
disconnect_btn.pack(side=LEFT, padx=(6, 10))

status_var = ctk.StringVar(value="Disconnected")
# PACK STATUS LABEL TO THE LEFT SO ITS NEXT TO DISCONNECT BUTTON
status_lbl = ctk.CTkLabel(com_frame, textvariable=status_var, text_color="gray70", font=FONT_SMALL)
status_lbl.pack(side=LEFT, padx=(6, 10), pady=(0, 8))


# ---------------- BATTERY + PICKUP/DROP REVISED BLOCK ----------------
battery_group = ctk.CTkFrame(right_panel, fg_color="#141414", corner_radius=8)
battery_group.pack(fill=X, padx=8, pady=(0, 8))

# Title
ctk.CTkLabel(battery_group, text="BATTERY / ACTIONS", font=FONT_TITLE).pack(anchor="w", padx=10, pady=(8, 4))

battery_inner = ctk.CTkFrame(battery_group, fg_color="#141414")
battery_inner.pack(fill=X, padx=10, pady=(0, 12))

# Layout:  Battery (large square) | Two small tiles (top row) | Two small tiles (bottom row)
battery_inner.columnconfigure(0, weight=2)   # big square
battery_inner.columnconfigure(1, weight=1)
battery_inner.columnconfigure(2, weight=1)

# ---- Battery (large square) ----
battery_tile = ctk.CTkFrame(battery_inner, fg_color="#1a1a1a", corner_radius=10)
battery_tile.grid(row=0, column=0, rowspan=2, sticky="nsew", padx=(0, 12), pady=6)

ctk.CTkLabel(battery_tile, text="BATTERY", font=FONT_SUBTITLE).pack(pady=(18, 4))
batt_pct_var = ctk.StringVar(value="-%")        # no space before %
batt_volt_var = ctk.StringVar(value="Voltage: - V")

# keep a reference to the label so we can change its color later
batt_pct_lbl = ctk.CTkLabel(
    battery_tile,
    textvariable=batt_pct_var,
    font=FONT_TILE_VALUE_LARGE,
    text_color="gray70"      # initial color, will be overridden
)
batt_pct_lbl.pack(pady=(10, 4))

ctk.CTkLabel(battery_tile, textvariable=batt_volt_var, font=FONT_SMALL).pack(pady=(0, 12))


# ---- Top Right Row: Left Encoder Degree + Pickup ----
left_deg_box = ctk.CTkFrame(battery_inner, fg_color="#1a1a1a", corner_radius=8)
left_deg_box.grid(row=0, column=1, sticky="nsew", padx=6, pady=6)
ctk.CTkLabel(left_deg_box, text="ENC LEFT DEG", font=FONT_TILE_TITLE).pack(pady=(8, 2))
left_deg_small_lbl = ctk.CTkLabel(left_deg_box, text="0.00°",
                                  font=FONT_VALUE_SMALL,
                                  text_color="#ffcc00",
                                  width=fs(110), anchor="center")
left_deg_small_lbl.pack(pady=(0, 8))
left_deg_box.grid_propagate(False)


pickup_tile = ctk.CTkFrame(battery_inner, fg_color="#1a1a1a", corner_radius=8)
pickup_tile.grid(row=0, column=2, sticky="nsew", padx=6, pady=6)
ctk.CTkLabel(pickup_tile, text="PICK UP BLOCK", font=FONT_TILE_TITLE).pack(pady=(8, 2))
pickup_val = ctk.CTkLabel(pickup_tile, text="-",
                          font=FONT_VALUE_MED_SMALL,
                          text_color="#ffcc00",
                          width=fs(110), anchor="center")
pickup_val.pack(pady=(0, 8))
pickup_tile.grid_propagate(False)


# ---- Bottom Right Row: Right Encoder Degree + Drop ----
right_deg_box = ctk.CTkFrame(battery_inner, fg_color="#1a1a1a", corner_radius=8)
right_deg_box.grid(row=1, column=1, sticky="nsew", padx=6, pady=6)
ctk.CTkLabel(right_deg_box, text="ENC RIGHT DEG", font=FONT_TILE_TITLE).pack(pady=(8, 2))
right_deg_small_lbl = ctk.CTkLabel(right_deg_box, text="0.00°",
                                   font=FONT_VALUE_SMALL,
                                   text_color="#ffcc00",
                                   width=fs(110), anchor="center")
right_deg_small_lbl.pack(pady=(0, 8))
right_deg_box.grid_propagate(False)


drop_tile = ctk.CTkFrame(battery_inner, fg_color="#1a1a1a", corner_radius=8)
drop_tile.grid(row=1, column=2, sticky="nsew", padx=6, pady=6)
ctk.CTkLabel(drop_tile, text="DROP BLOCK", font=FONT_TILE_TITLE).pack(pady=(8, 2))
drop_val = ctk.CTkLabel(drop_tile, text="-",
                        font=FONT_VALUE_MED_SMALL,
                        text_color="#ffcc00",
                        width=fs(110), anchor="center")
drop_val.pack(pady=(0, 8))
drop_tile.grid_propagate(False)


# ---------------- APRILTAG / ESP block (improved layout) ----------------
april_big = ctk.CTkFrame(right_panel, fg_color="#141414", corner_radius=8)
april_big.pack(fill=BOTH, expand=True, padx=14, pady=(0, 8))   # shifted slightly right

ctk.CTkLabel(april_big, text="APRIL TAG / ESP DATA", font=FONT_TITLE).pack(padx=10, pady=(10, 6))

april_inner = ctk.CTkFrame(april_big, fg_color="#141414")
april_inner.pack(fill=BOTH, expand=True, padx=10, pady=(0, 10))

# -------- TAG ID (left big tile) --------
tag_frame = ctk.CTkFrame(april_inner, fg_color="#1a1a1a", corner_radius=10)
tag_frame.grid(row=0, column=0, rowspan=3, sticky="nsew", padx=(0, 12), pady=6)

ctk.CTkLabel(tag_frame, text="TAG ID", font=FONT_TAG_TITLE).pack(pady=(20, 10))

tag_var = ctk.StringVar(value="0")

tag_lbl = ctk.CTkLabel(
    tag_frame,
    textvariable=tag_var,
    font=FONT_TAG_VALUE,
    text_color="#ffcc00",
    width=fs(180),      # Slightly larger box for big text
    anchor="center"
)
tag_lbl.pack(pady=(20, 20))


tag_frame.grid_propagate(False)

# Make the TAG ID tile wider
april_inner.columnconfigure(0, weight=2)   # bigger tile
april_inner.columnconfigure(1, weight=1)
april_inner.columnconfigure(2, weight=1)

# -------- ESP small tiles (square-like) --------
def make_esp_tile(parent, title, var):
    f = ctk.CTkFrame(parent, fg_color="#1a1a1a", corner_radius=8)
    ctk.CTkLabel(f, text=title, font=FONT_TILE_TITLE).pack(pady=(8, 2))
    val = ctk.CTkLabel(
        f,
        textvariable=var,
        font=FONT_VALUE_SMALL,
        text_color="#ffcc00",
        width=fs(110),
        anchor="center"
    )
    val.pack(pady=(0, 8))

    f.grid_propagate(False)
    return f

pos_x_var = ctk.StringVar(value="0.00")
pos_y_var = ctk.StringVar(value="0.00")
pos_z_var = ctk.StringVar(value="0.00")
yaw_val_var = ctk.StringVar(value="0.00")
roll_val_var = ctk.StringVar(value="0.00")
pitch_val_var = ctk.StringVar(value="0.00")

# X AXIS
f = make_esp_tile(april_inner, "X AXIS", pos_x_var)
f.grid(row=0, column=1, padx=6, pady=6, sticky="nsew")

# YAW
f = make_esp_tile(april_inner, "YAW", yaw_val_var)
f.grid(row=0, column=2, padx=6, pady=6, sticky="nsew")

# Y AXIS
f = make_esp_tile(april_inner, "Y AXIS", pos_y_var)
f.grid(row=1, column=1, padx=6, pady=6, sticky="nsew")

# ROLL
f = make_esp_tile(april_inner, "ROLL", roll_val_var)
f.grid(row=1, column=2, padx=6, pady=6, sticky="nsew")

# Z AXIS
f = make_esp_tile(april_inner, "Z AXIS", pos_z_var)
f.grid(row=2, column=1, padx=6, pady=6, sticky="nsew")

# PITCH
f = make_esp_tile(april_inner, "PITCH", pitch_val_var)
f.grid(row=2, column=2, padx=6, pady=6, sticky="nsew")

# Make rows equal height
april_inner.rowconfigure(0, weight=1)
april_inner.rowconfigure(1, weight=1)
april_inner.rowconfigure(2, weight=1)


# ------------- Bottom Controls (includes NEW Yaw Scale Dropdown) ----------------
bottom_row = ctk.CTkFrame(right_panel, fg_color="#101010")
bottom_row.pack(fill=X, padx=8, pady=(0, 10))

start_btn = ctk.CTkButton(
    bottom_row,
    text="Start Logging",
    width=120,
    font=FONT_BUTTON,
    fg_color="transparent",
    border_color="#22c55e",  # green border
    border_width=2,
    hover_color=("gray25")
)

stop_btn = ctk.CTkButton(
    bottom_row,
    text="Stop Logging",
    width=120,
    font=FONT_BUTTON,
    fg_color="transparent",
    border_color="#ef4444",  # red border
    border_width=2,
    hover_color=("gray25"),
    state="disabled"
)

start_btn.pack(side=LEFT, padx=6, pady=10)
stop_btn.pack(side=LEFT, padx=6, pady=10)

csv_chk = ctk.CTkCheckBox(bottom_row, text="Log to CSV", font=FONT_SMALL)
csv_chk.select()
write_to_csv_flag.set()
csv_chk.pack(side=LEFT, padx=6, pady=10)


# ---------------- Yaw Scale dropdown ----------------
def on_scale_preset_changed(choice):
    global yaw_auto_scale, yaw_manual_min, yaw_manual_max
    if choice == "Auto":
        yaw_auto_scale = True
    else:
        yaw_auto_scale = False
        parts = choice.replace("+", "").split("to")
        try:
            yaw_manual_min = float(parts[0].strip())
            yaw_manual_max = float(parts[1].strip())
        except:
            yaw_manual_min, yaw_manual_max = -180, 180


yaw_scale_menu = ctk.CTkOptionMenu(
    bottom_row,
    values=yaw_scale_presets,
    variable=yaw_scale_var,
    width=150,
    command=on_scale_preset_changed,
    font=FONT_SMALL
)
yaw_scale_menu.pack(side=RIGHT, padx=8, pady=10)

# Initialize scale
on_scale_preset_changed(yaw_scale_var.get())


# ---------------- Thread Management ----------------
serial_thread = None
csv_thread = None


def csv_checkbox_changed():
    write_to_csv_flag.set() if csv_chk.get() == 1 else write_to_csv_flag.clear()


csv_chk.configure(command=csv_checkbox_changed)


def safe_start_serial(port, baud):
    global serial_thread
    serial_stop_event.clear()
    serial_thread = threading.Thread(target=serial_reader_thread, args=(port, baud), daemon=True)
    serial_thread.start()


def connect_action():
    port = com_var.get()
    try:
        baud = int(baud_var.get())
    except:
        baud = DEFAULT_BAUD

    safe_start_serial(port, baud)
    status_var.set(f"Connecting {port}...")
    connect_btn.configure(state="disabled")
    disconnect_btn.configure(state="normal")

    if csv_thread is None:
        start_csv_thread()


def disconnect_action():
    serial_stop_event.set()
    time.sleep(0.15)
    connect_btn.configure(state="normal")
    disconnect_btn.configure(state="disabled")
    status_var.set("Disconnected")


def start_logging_action():
    logging_enabled.set()
    start_btn.configure(state="disabled")
    stop_btn.configure(state="normal")


def stop_logging_action():
    logging_enabled.clear()
    start_btn.configure(state="normal")
    stop_btn.configure(state="disabled")


connect_btn.configure(command=connect_action)
disconnect_btn.configure(command=disconnect_action)
start_btn.configure(command=start_logging_action)
stop_btn.configure(command=stop_logging_action)


def start_csv_thread():
    global csv_thread
    csv_thread = threading.Thread(target=csv_logger_thread, args=(CSV_LOG_PATH,), daemon=True)
    csv_thread.start()

fb_thread = threading.Thread(target=firebase_listener_thread, daemon=True)
fb_thread.start()

# ---------------- Serial Reader Thread ----------------
def serial_reader_thread(port_name, baud):
    global ser
    buffer = ""
    try:
        with serial_lock:
            ser = serial.Serial(port_name, baud, timeout=0)
        # Mark connected and update UI status immediately
        serial_connected.set()
        try:
            status_var.set("Connected")
        except Exception:
            # status_var might not exist in rare race conditions; ignore
            pass
        print(f"[Serial] Opened {port_name} @ {baud}")
    except Exception as e:
        serial_connected.clear()
        try:
            status_var.set("Disconnected")
        except Exception:
            pass
        print("[Serial] Open failed:", e)
        return

    while not serial_stop_event.is_set():
        try:
            with serial_lock:
                if ser is None:
                    break
                available = ser.in_waiting
                chunk = ser.read(ser.in_waiting or 1).decode('utf-8', errors='ignore')
                buffer += chunk
        except Exception:
            break

        if chunk:
            buffer += chunk

        while "\n" in buffer:
            line, buffer = buffer.split("\n", 1)
            line = line.strip()

            # Process complete newline-terminated JSON packets
            while "\n" in buffer:
                packet, buffer = buffer.split("\n", 1)
                packet = packet.strip()
                if not packet:
                    continue

                try:
                    data = json.loads(packet)
                except json.JSONDecodeError:
                    print("Bad JSON (skipped):", packet)
                    continue

                gui_queue.put_nowait(data)

            try:
                data = json.loads(line)
            except Exception as e:
                print("JSON parse error:", e, "| LINE:", line)
                continue

            # push latest to GUI
            try:
                gui_queue.put_nowait(data)
            except queue.Full:
                try:
                    gui_queue.get_nowait()
                    gui_queue.put_nowait(data)
                except:
                    pass

            # CSV logging
            if logging_enabled.is_set() and write_to_csv_flag.is_set():
                try:
                    log_queue.put_nowait(prepare_row_for_csv(data))
                except queue.Full:
                    try:
                        log_queue.get_nowait()
                        log_queue.put_nowait(prepare_row_for_csv(data))
                    except:
                        pass

            # push latest to GUI
            try:
                gui_queue.put_nowait(data)
            except queue.Full:
                try:
                    _ = gui_queue.get_nowait()
                    gui_queue.put_nowait(data)
                except:
                    pass

            if logging_enabled.is_set() and write_to_csv_flag.is_set():
                try:
                    log_queue.put_nowait(prepare_row_for_csv(data))
                except queue.Full:
                    try:
                        _ = log_queue.get_nowait()
                        log_queue.put_nowait(prepare_row_for_csv(data))
                    except:
                        pass

        time.sleep(0.001)

    # cleanup: close serial and update status
    with serial_lock:
        try:
            if ser and ser.is_open:
                ser.close()
        except:
            pass
        ser = None
    serial_connected.clear()
    try:
        status_var.set("Disconnected")
    except Exception:
        pass
    print("[Serial] Reader exiting")


# ---------------- CSV Logger Thread ----------------
def csv_logger_thread(path):
    header = [
        "Timestamp", "Left", "Right", "Left_deg", "Right_deg",
        "Accel_X", "Accel_Y", "Accel_Z",
        "Gyro_X", "Gyro_Y", "Gyro_Z",
        "Pitch", "Roll", "Yaw",
        "Battery_V", "Battery_Percent",
        "ESP_Tag_ID", "ESP_Yaw", "ESP_Pitch", "ESP_Roll",
        "ESP_X", "ESP_Y", "ESP_Z"
    ]

    first_needed = not os.path.exists(path)
    try:
        f = open(path, "a", newline="", encoding="utf-8")
        writer = csv.writer(f)
        if first_needed:
            writer.writerow(header)
            f.flush()
    except Exception as e:
        print("[CSV] Cannot open log file:", e)
        return

    buffer_rows = []
    last_flush = time.time()

    while not (serial_stop_event.is_set() and log_queue.empty() and not logging_enabled.is_set()):
        try:
            while True:
                buffer_rows.append(log_queue.get_nowait())
                if len(buffer_rows) >= 600:
                    break
        except queue.Empty:
            pass

        now = time.time()
        if buffer_rows and (now - last_flush >= CSV_BATCH_INTERVAL):
            try:
                writer.writerows(buffer_rows)
                f.flush()
            except Exception as e:
                print("[CSV] Write error:", e)
            buffer_rows = []
            last_flush = now

        time.sleep(0.05)

    if buffer_rows:
        try:
            writer.writerows(buffer_rows)
            f.flush()
        except Exception:
            pass

    try:
        f.close()
    except Exception:
        pass


# ---------------- GUI Update Loop ----------------
latest_batt_voltage = 0.0


def update_gui_from_queue():
    global latest_batt_voltage
    latest = None
    while True:
        try:
            latest = gui_queue.get_nowait()
        except queue.Empty:
            break

    if latest:
        enc = latest.get("enc", {})
        imu = latest.get("imu", {})
        battery = latest.get("battery", {})
        esp = latest.get("esp", {})

        # ----- SAFE ENCODER UPDATE -----
        try:
            enc = latest.get("enc") or {}

            L = safe_int(enc.get("L"))
            R = safe_int(enc.get("R"))
            Ld = safe_float(enc.get("left_deg"))
            Rd = safe_float(enc.get("right_deg"))

            left_enc_lbl.configure(text=str(L))
            right_enc_lbl.configure(text=str(R))
            left_deg_lbl.configure(text=f"[{Ld:.2f}°]")
            right_deg_lbl.configure(text=f"[{Rd:.2f}°]")
            left_deg_small_lbl.configure(text=f"{Ld:.2f}°")
            right_deg_small_lbl.configure(text=f"{Rd:.2f}°")

        except Exception as e:
            print("ENCODER UPDATE ERROR:", e)

        # imu
        acc = pad_list(imu.get("acc", []), 3)
        gyro = pad_list(imu.get("gyro", []), 3)
        euler = pad_list(imu.get("euler", []), 3)

        imu_tiles["Accel X (g)"].configure(text=f"{acc[0]:.2f}")
        imu_tiles["Accel Y (g)"].configure(text=f"{acc[1]:.2f}")
        imu_tiles["Accel Z (g)"].configure(text=f"{acc[2]:.2f}")
        imu_tiles["Gyro X (°/s)"].configure(text=f"{gyro[0]:.2f}")
        imu_tiles["Gyro Y (°/s)"].configure(text=f"{gyro[1]:.2f}")
        imu_tiles["Gyro Z (°/s)"].configure(text=f"{gyro[2]:.2f}")
        imu_tiles["Pitch"].configure(text=f"{euler[0]:.2f}")
        imu_tiles["Roll"].configure(text=f"{euler[1]:.2f}")
        imu_tiles["Yaw"].configure(text=f"{euler[2]:.2f}")

        # yaw graph
        yaw_val = euler[2]
        if yaw_val > 180:
            yaw_val -= 360
        elif yaw_val < -180:
            yaw_val += 360

        yaw_history.append(yaw_val)
        if len(yaw_history) > MAX_POINTS:
            yaw_history.pop(0)

        ax.cla()
        ax.set_facecolor("black")
        ax.grid(True, linestyle="--", linewidth=0.5, alpha=0.4)
        # plot line without forcing color constants (matplotlib default)
        ax.plot(yaw_history, linewidth=2)
        ax.tick_params(colors="white")
        ax.set_title("Yaw (recent)", color="white", fontsize=fs(10))

        # apply scaling
        if yaw_auto_scale and yaw_history:
            ymin = min(yaw_history) - YAW_SCALE_MARGIN
            ymax = max(yaw_history) + YAW_SCALE_MARGIN
            if abs(ymax - ymin) < 2:
                ymin -= 10
                ymax += 10
        else:
            ymin, ymax = yaw_manual_min, yaw_manual_max

        ax.set_ylim(ymin, ymax)
        ax.set_xlim(0, MAX_POINTS)

        yaw_canvas.draw()

        # AprilTag & battery
        tag_var.set(str(safe_int(esp.get("tag", 0))))
        pos = esp.get("pos", [0, 0, 0]) if isinstance(esp.get("pos", None), list) else [0, 0, 0]

        pos_x_var.set(f"{safe_float(pos[0]):.2f}")
        pos_y_var.set(f"{safe_float(pos[1]):.2f}")
        pos_z_var.set(f"{safe_float(pos[2]):.2f}")

        # --- ESP AprilTag orientation (correct source) ---
        yaw_val_var.set(f"{safe_float(esp.get('yaw', 0.0)):.2f}")
        pitch_val_var.set(f"{safe_float(esp.get('pitch', 0.0)):.2f}")
        roll_val_var.set(f"{safe_float(esp.get('roll', 0.0)):.2f}")

        latest_batt_voltage = safe_float(battery.get("voltage", latest_batt_voltage))

        pct = safe_int(battery.get("percent", 0))
        batt_pct_var.set(f"{pct}%")  # <<< no space before %
        try:
            db.reference("/AUTOBOT/AUTOBOT/Battery").set(f"{pct}%")
        except Exception as e:
            print("Battery Firebase update failed:", e)

        # set color based on percentage
        if pct >= 75:
            color = "#22c55e"  # green
        elif pct >= 50:
            color = "#eab308"  # yellow
        elif pct >= 25:
            color = "#f97316"  # orange
        else:
            color = "#ef4444"  # red

        batt_pct_lbl.configure(text_color=color)

        batt_volt_var.set(f"Voltage: {latest_batt_voltage:.2f} V")

        if "pickup" in latest:
            pickup_val.configure(text=str(latest["pickup"]))
        if "drop" in latest:
            drop_val.configure(text=str(latest["drop"]))

    # update connection status
    status_lbl.configure(text="Connected" if serial_connected.is_set() else "Disconnected")

    app.after(GUI_POLL_MS, update_gui_from_queue)


# ---------------- Shutdown ----------------
def on_closing():
    serial_stop_event.set()
    logging_enabled.clear()
    write_to_csv_flag.clear()
    time.sleep(0.2)
    app.destroy()


app.protocol("WM_DELETE_WINDOW", on_closing)


# Start
app.after(100, update_gui_from_queue)
app.mainloop()
