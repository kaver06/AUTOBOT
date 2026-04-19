import serial
import time
import os
import json
import win32com.client as win32

# ==== USER SETTINGS ====
COM_PORT = 'COM12'
BAUD_RATE = 115200
EXCEL_PATH = r'C:\Users\kaver\OneDrive\Desktop\C_files\Python\AutoBot\Autobot_Log.xlsx'
SHEET_NAME = 'Data'
# ========================

# Ensure folder exists
folder = os.path.dirname(EXCEL_PATH)
os.makedirs(folder, exist_ok=True)

# Serial port (non-blocking read)
ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=0)
print(f"Connected to {COM_PORT} at {BAUD_RATE} baud.")
time.sleep(2)

# Excel application
try:
    excel = win32.GetObject(None, "Excel.Application")
except:
    excel = win32.Dispatch("Excel.Application")
excel.Visible = True

# Workbook
try:
    wb = excel.Workbooks.Open(EXCEL_PATH)
except:
    wb = excel.Workbooks.Add()
    wb.SaveAs(EXCEL_PATH, FileFormat=51)

# Sheet
try:
    ws = wb.Sheets(SHEET_NAME)
except:
    ws = wb.Sheets.Add()
    ws.Name = SHEET_NAME

# Clear old content
ws.Cells.Clear()

# Columns / headers
headers = [
    "Timestamp",
    "Left", "Right", "Left_deg", "Right_deg",
    "Accel_X", "Accel_Y", "Accel_Z",
    "Gyro_X", "Gyro_Y", "Gyro_Z",
    "Pitch", "Roll", "Yaw",
    "Battery_V", "Battery_%",
    "ESP_Tag_ID",
    "ESP_Yaw", "ESP_Pitch", "ESP_Roll",
    "ESP_X", "ESP_Y", "ESP_Z"
]

for col, text in enumerate(headers, start=1):
    c = ws.Cells(1, col)
    c.Value = text
    c.Font.Bold = True
    c.Interior.Color = 0x00B050
    c.Font.Color = 0xFFFFFF
    c.HorizontalAlignment = -4108
    c.VerticalAlignment = -4108

# Freeze header
ws.Rows("2:2").Select()
excel.ActiveWindow.FreezePanes = True

# Globals
row = 2
buffer = ""

# ---------------- UTILITY FUNCTIONS ----------------

def pad_list(arr, n=3):
    """Return list of length n safely (pads with 0.0)."""
    if not isinstance(arr, list):
        return [0.0] * n
    out = arr[:n] + [0.0] * max(0, n - len(arr))
    # ensure numeric conversion where possible
    for i in range(len(out)):
        try:
            out[i] = float(out[i])
        except Exception:
            out[i] = 0.0
    return out

def excel_busy():
    try:
        return not excel.Application.Interactive
    except:
        return True

def write_row(data):
    global row
    try:
        ws.Range(ws.Cells(row, 1), ws.Cells(row, len(data))).Value = [data]
        row += 1
        return True
    except Exception as e:
        # don't crash on Excel COM errors
        print("Excel write failed:", e)
        return False

def auto_fit_columns():
    try:
        ws.Columns.AutoFit()
    except:
        pass

def battery_color_format(row_num, percent):
    try:
        cell = ws.Cells(row_num, 16)  # Battery_Percent column
        # percent might not be numeric
        try:
            p = float(percent)
        except:
            p = 0.0
        if p >= 75:
            cell.Interior.Color = 0x00FF00  # Green
        elif p >= 40:
            cell.Interior.Color = 0x00FFFF  # Yellow
        else:
            cell.Interior.Color = 0xFF0000  # Red
    except:
        pass

def imu_format(row_num, euler):
    # euler is list of three floats
    try:
        pitch, roll, yaw = euler
    except:
        pitch = roll = yaw = 0.0

    def highlight(col_index, val):
        try:
            if abs(float(val)) > 25:
                ws.Cells(row_num, col_index).Interior.Color = 0x9999FF
        except:
            pass

    # Pitch column index = 12, Roll = 13, Yaw = 14 (1-based)
    highlight(12, pitch)
    highlight(13, roll)
    highlight(14, yaw)

def auto_scroll_one_step():
    """Scroll only AFTER 75% of visible rows, and only ONE row each time."""
    try:
        ws.Activate()
        win = excel.ActiveWindow
        visible_rows = int(win.VisibleRange.Rows.Count)
        # if visible_rows is zero for whatever reason, skip
        if visible_rows <= 0:
            return
        threshold = int(visible_rows * 0.75)
        # begin smooth incremental scroll
        if row > threshold:
            # ensure we don't increment beyond available sheet rows
            try:
                current_scroll = int(win.ScrollRow)
            except:
                current_scroll = 1
            win.ScrollRow = max(1, current_scroll + 1)
    except Exception:
        pass

def safe_save():
    try:
        wb.Save()
    except:
        pass

# ---------------- MAIN LOOP ----------------

print("\nJSON logger with smooth incremental scrolling started...\n")

try:
    while True:
        # Read available bytes (non-blocking)
        incoming = ser.read(ser.in_waiting or 1).decode('utf-8', errors='ignore')
        if incoming:
            buffer += incoming

        # Process complete newline-terminated JSON packets
        while "\n" in buffer:
            packet, buffer = buffer.split("\n", 1)
            packet = packet.strip()
            if not packet:
                continue

            # Try to parse JSON; skip invalid JSON safely
            try:
                data = json.loads(packet)
            except json.JSONDecodeError:
                print("Bad JSON (skipped):", packet)
                continue

            timestamp = time.strftime("%Y-%m-%d %H:%M:%S")

            enc = data.get("enc", {})
            imu = data.get("imu", {})
            bat = data.get("battery", {})
            esp = data.get("esp", {})

            # Safe padded arrays
            e_acc = pad_list(imu.get("acc", []), 3)
            e_gyro = pad_list(imu.get("gyro", []), 3)
            e_euler = pad_list(imu.get("euler", []), 3)

            # Safe ESP pos
            raw_pos = esp.get("pos", [0, 0, 0])
            if isinstance(raw_pos, list):
                pos = raw_pos[:3] + [0.0] * max(0, 3 - len(raw_pos))
                # convert to floats safely
                for i in range(3):
                    try:
                        pos[i] = float(pos[i])
                    except:
                        pos[i] = 0.0
            else:
                pos = [0.0, 0.0, 0.0]

            # row data (all values coerced where possible)
            def safe_get_num(obj, key, default=0.0):
                v = obj.get(key, default)
                try:
                    return float(v)
                except:
                    return default

            def safe_get_int(obj, key, default=0):
                v = obj.get(key, default)
                try:
                    return int(v)
                except:
                    try:
                        return int(float(v))
                    except:
                        return default

            row_data = [
                timestamp,
                # enc
                safe_get_int(enc, "L", 0),
                safe_get_int(enc, "R", 0),
                safe_get_num(enc, "left_deg", 0.0),
                safe_get_num(enc, "right_deg", 0.0),
                # acc
                e_acc[0], e_acc[1], e_acc[2],
                # gyro
                e_gyro[0], e_gyro[1], e_gyro[2],
                # euler
                e_euler[0], e_euler[1], e_euler[2],
                # battery
                safe_get_num(bat, "voltage", 0.0),
                safe_get_int(bat, "percent", 0),
                # esp
                safe_get_int(esp, "tag", 0),
                safe_get_num(esp, "yaw", 0.0),
                safe_get_num(esp, "pitch", 0.0),
                safe_get_num(esp, "roll", 0.0),
                # pos
                pos[0], pos[1], pos[2],
            ]

            # print for console debugging
            print(row_data)

            # write to excel if Excel is interactive
            if not excel_busy():
                if write_row(row_data):
                    # formatting and scrolling using the newly written row
                    battery_color_format(row - 1, bat.get("percent", 0))
                    imu_format(row - 1, e_euler)
                    auto_scroll_one_step()
                    auto_fit_columns()
                    safe_save()

        # small sleep to avoid 100% CPU
        time.sleep(0.01)

except KeyboardInterrupt:
    print("\nStopped by user.")
    safe_save()
    ser.close()
    print("Closed cleanly.")
