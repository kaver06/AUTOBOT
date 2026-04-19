import customtkinter as ctk
from tkinter import *
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

# ---------------- WINDOW SETUP ----------------
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")

root = ctk.CTk()
root.title("AUTOBOT GUI")
root.geometry("1280x720")
root.configure(fg_color="black")

# ---------------- HEADER ----------------
header_frame = ctk.CTkFrame(root, fg_color="black")
header_frame.pack(fill="x", pady=(10, 5))

title_lbl = ctk.CTkLabel(
    header_frame,
    text="AUTOBOT",
    font=("Agency FB", 45, "bold"),
    text_color="gray80"
)
title_lbl.pack()

subtitle_lbl = ctk.CTkLabel(
    header_frame,
    text="AUTOMATING YOUR EVERYDAY TASK",
    font=("Segoe UI", 14),
    text_color="gray80"
)
subtitle_lbl.pack()

# ---------------- MAIN CONTAINER ----------------
main_frame = ctk.CTkFrame(root, fg_color="black")
main_frame.pack(fill="both", expand=True, padx=20, pady=10)

# ---------------- LEFT SIDE ----------------
left_frame = ctk.CTkFrame(main_frame, fg_color="black")
left_frame.pack(side="left", fill="both", expand=True, padx=10)

# Yaw Live Graph
yaw_frame = ctk.CTkFrame(left_frame, fg_color="#1a1a1a", corner_radius=8)
yaw_frame.pack(pady=10, padx=10, fill="both", expand=True)

yaw_lbl = ctk.CTkLabel(
    yaw_frame, text="YAW LIVE GRAPH",
    font=("Segoe UI", 14, "bold"), text_color="white"
)
yaw_lbl.pack(pady=(8, 0))

fig = Figure(figsize=(4.5, 2.5), dpi=100)
ax = fig.add_subplot(111)
ax.set_facecolor("black")
ax.tick_params(colors="white")
fig.patch.set_facecolor("black")
canvas = FigureCanvasTkAgg(fig, master=yaw_frame)
canvas.get_tk_widget().pack(fill="both", expand=True, pady=5)

# IMU Panels (2x2 grid)
imu_grid = ctk.CTkFrame(left_frame, fg_color="black")
imu_grid.pack(fill="both", expand=True, padx=10, pady=10)

imu_values = {}
axes = [("X AXIS GYRO", "#2a2a2a"), ("Y AXIS GYRO", "#2a2a2a"),
        ("X AXIS ACCEL", "#2a2a2a"), ("Z AXIS ACCEL", "#2a2a2a")]

for i, (label, color) in enumerate(axes):
    f = ctk.CTkFrame(imu_grid, fg_color=color, corner_radius=6)
    f.grid(row=i//2, column=i%2, padx=10, pady=10, sticky="nsew")
    lbl = ctk.CTkLabel(f, text=label, font=("Segoe UI", 16, "bold"), text_color="white")
    lbl.pack(pady=(10, 5))
    val = ctk.CTkLabel(f, text="0.00", font=("Consolas", 26, "bold"), text_color="#ffcc00")
    val.pack(pady=(0, 10))
    imu_values[label] = val

imu_grid.columnconfigure((0, 1), weight=1)
imu_grid.rowconfigure((0, 1), weight=1)

# ---------------- RIGHT SIDE ----------------
right_frame = ctk.CTkFrame(main_frame, fg_color="black")
right_frame.pack(side="right", fill="both", expand=True, padx=10)

# ---------------- COM + Battery + Pickup/Drop ----------------
top_status = ctk.CTkFrame(right_frame, fg_color="black")
top_status.pack(fill="x", pady=10)

# COM PORT SECTION
com_frame = ctk.CTkFrame(top_status, fg_color="#252525", corner_radius=8)
com_frame.pack(side="left", padx=10, ipadx=15, ipady=10, fill="both", expand=True)

ctk.CTkLabel(
    com_frame,
    text="COM PORT STATUS:",
    font=("Segoe UI", 14, "bold"),
    text_color="white",
    anchor="w",
    justify="left"
).pack(fill="x", padx=15, pady=(5, 10))

blocks_frame = ctk.CTkFrame(com_frame, fg_color="#252525")
blocks_frame.pack(fill="x", padx=10, pady=5)

pickup_frame = ctk.CTkFrame(blocks_frame, fg_color="#2e2e2e", corner_radius=6)
pickup_frame.pack(side="left", expand=True, fill="both", padx=8, pady=5)

ctk.CTkLabel(
    pickup_frame, text="PICK UP BLOCK",
    font=("Segoe UI", 13, "bold"), text_color="white"
).pack(pady=(10, 2))

pickup_val = ctk.CTkLabel(
    pickup_frame, text="-",
    font=("Consolas", 50, "bold"), text_color="white"
)
pickup_val.pack(pady=(0, 10))

drop_frame = ctk.CTkFrame(blocks_frame, fg_color="#2e2e2e", corner_radius=6)
drop_frame.pack(side="right", expand=True, fill="both", padx=8, pady=5)

ctk.CTkLabel(
    drop_frame, text="DROP BLOCK",
    font=("Segoe UI", 13, "bold"), text_color="white"
).pack(pady=(10, 2))

drop_val = ctk.CTkLabel(
    drop_frame, text="-",
    font=("Consolas", 50, "bold"), text_color="white"
)
drop_val.pack(pady=(0, 10))

# Battery Section
battery_frame = ctk.CTkFrame(top_status, fg_color="#252525", corner_radius=8)
battery_frame.pack(side="right", padx=10, ipadx=15, ipady=10, fill="both", expand=True)

ctk.CTkLabel(
    battery_frame, text="BATTERY",
    font=("Segoe UI", 14, "bold"), text_color="white"
).pack(pady=(10, 2))

battery_val = ctk.CTkLabel(
    battery_frame, text="- %",
    font=("Consolas", 58, "bold"), text_color="#00ff00"
)
battery_val.pack(pady=(0, 5))

battery_voltage = ctk.CTkLabel(
    battery_frame, text="Voltage: 0V",
    font=("Consolas", 20, "bold"), text_color="gray90"
)
battery_voltage.pack(pady=(0, 10))

# ---------------- APRIL TAG / ESP DATA SECTION ----------------
april_frame = ctk.CTkFrame(right_frame, fg_color="#1a1a1a", corner_radius=10)
april_frame.pack(fill="both", expand=True, padx=10, pady=10)

april_lbl = ctk.CTkLabel(
    april_frame, text="APRIL TAG / ESP DATA",
    font=("Segoe UI", 18, "bold"), text_color="white"
)
april_lbl.pack(pady=(8, 0))

april_grid = ctk.CTkFrame(april_frame, fg_color="#1a1a1a")
april_grid.pack(fill="both", expand=True, pady=10, padx=10)

april_data_labels = {}

# Left column - Tag ID
tag_frame = ctk.CTkFrame(april_grid, fg_color="#2a2a2a", corner_radius=8)
tag_frame.grid(row=0, column=0, rowspan=3, padx=10, pady=10, sticky="nsew")

tag_lbl = ctk.CTkLabel(
    tag_frame, text="TAG ID",
    font=("Segoe UI", 16, "bold"), text_color="white"
)
tag_lbl.pack(pady=(10, 5))

tag_val = ctk.CTkLabel(
    tag_frame, text="0",
    font=("Consolas", 70, "bold"),
    text_color="#ffcc00"
)
tag_val.pack(expand=True, fill="both")
april_data_labels["TAG ID"] = tag_val

# Middle column - X Y Z
axes = ["X AXIS", "Y AXIS", "Z AXIS"]
for i, name in enumerate(axes):
    f = ctk.CTkFrame(april_grid, fg_color="#2a2a2a", corner_radius=8)
    f.grid(row=i, column=1, padx=10, pady=10, sticky="nsew")
    lbl = ctk.CTkLabel(f, text=name, font=("Segoe UI", 14, "bold"), text_color="white")
    lbl.pack(pady=(10, 2))
    val = ctk.CTkLabel(f, text="0.00", font=("Consolas", 22, "bold"), text_color="#ffcc00")
    val.pack(pady=(0, 10))
    april_data_labels[name] = val

# Right column - Yaw Roll Pitch
orient = ["YAW", "ROLL", "PITCH"]
for i, name in enumerate(orient):
    f = ctk.CTkFrame(april_grid, fg_color="#2a2a2a", corner_radius=8)
    f.grid(row=i, column=2, padx=10, pady=10, sticky="nsew")
    lbl = ctk.CTkLabel(f, text=name, font=("Segoe UI", 14, "bold"), text_color="white")
    lbl.pack(pady=(10, 2))
    val = ctk.CTkLabel(f, text="0.00", font=("Consolas", 22, "bold"), text_color="#ffcc00")
    val.pack(pady=(0, 10))
    april_data_labels[name] = val

for i in range(3):
    april_grid.rowconfigure(i, weight=1)
    april_grid.columnconfigure(i, weight=1)

# ---------------- START GUI ----------------
root.mainloop()
