import tkinter as tk
import serial
import serial.tools.list_ports
import threading
import time

class PointCounter:
    def __init__(self):
        self.dist = [999, 999, 999, 999]
        self.points = [0, 0, 0, 0]
        self.total = 0
        self.ser = None
        self.running = False
        
    def find_esp32(self):
        """หา ESP32 port อัตโนมัติ"""
        ports = serial.tools.list_ports.comports()
        for port in ports:
            if "ESP32" in port.description or "Silicon Labs" in port.description:
                return port.device
        return ports[-1].device if ports else None
    
    def connect(self):
        try:
            port = self.find_esp32()
            if not port:
                self.status = "❌ ไม่พบ ESP32"
                return False
                
            self.ser = serial.Serial(port, 115200, timeout=1)
            time.sleep(2)  # รอ ESP32 boot
            self.status = f"🟢 เชื่อมต่อ {port}"
            self.running = True
            threading.Thread(target=self.read_serial, daemon=True).start()
            return True
        except Exception as e:
            self.status = f"🔴 Error: {e}"
            return False
    
    def read_serial(self):
        while self.running:
            try:
                if self.ser and self.ser.in_waiting:
                    line = self.ser.readline().decode('utf-8', errors='ignore').strip()
                    if line.startswith("DATA,"):
                        parts = line.split(',')
                        if len(parts) == 10:
                            self.dist = [float(parts[i]) for i in range(1,5)]
                            self.points = [int(parts[i]) for i in range(5,9)]
                            self.total = int(parts[9])
                            self.update_gui()
                    elif line.startswith("HIT"):
                        print(f"🎯 {line}")
                    elif line == "RESET_OK":
                        self.points = [0, 0, 0, 0]
                        self.total = 0
                        self.update_gui()
            except:
                pass
            time.sleep(0.05)
    
    def reset(self):
        if self.ser:
            self.ser.write(b"RESET\n")
    
    def update_gui(self):
        # Distances
        for i in range(4):
            color = 'lime' if self.dist[i] < 30 else 'orange' if self.dist[i] < 100 else 'red'
            self.labels[i].config(text=f"S{i+1}: {self.dist[i]:.0f}cm", fg=color)
        
        # Points
        colors = ['lime', 'cyan', 'magenta', 'yellow']
        for i in range(4):
            self.p_labels[i].config(text=str(self.points[i]), fg=colors[i])
        
        self.total_label.config(text=f"TOTAL: {self.total}")

# GUI
app = PointCounter()
root = tk.Tk()
root.title("🎯 ESP32 USB Point Counter")
root.geometry("500x550")
root.configure(bg='black')

# Status
status_label = tk.Label(root, text="กำลังเชื่อมต่อ...", font=("Arial", 14), fg="yellow", bg="black")
status_label.pack(pady=10)

# Distance labels
tk.Label(root, text="📏 ระยะทาง", font=("Arial", 16, "bold"), fg="cyan", bg="black").pack(pady=5)
app.labels = []
for i in range(4):
    label = tk.Label(root, text="S1: 999cm", font=("Arial", 18), bg="black")
    label.pack(pady=3)
    app.labels.append(label)

# Points
tk.Label(root, text="🎯 คะแนน", font=("Arial", 16, "bold"), fg="gold", bg="black").pack(pady=10)
app.p_labels = []
colors = ['lime', 'cyan', 'magenta', 'yellow']
p_frame = tk.Frame(root, bg="black")
p_frame.pack(pady=10)
for i in range(4):
    p_label = tk.Label(p_frame, text="0", font=("Arial", 30, "bold"), 
                      fg=colors[i], bg="black", width=4)
    p_label.pack(side=tk.LEFT, padx=15)
    app.p_labels.append(p_label)

app.total_label = tk.Label(root, text="TOTAL: 0", font=("Arial", 28, "bold"), fg="gold", bg="black")
app.total_label.pack(pady=20)

# Buttons
button_frame = tk.Frame(root, bg="black")
button_frame.pack(pady=10)
tk.Button(button_frame, text="🔌 CONNECT", font=("Arial", 14), bg="green", fg="white",
          command=lambda: app.connect() or setattr(app, 'status', status_label.config(text=app.status))).pack(side=tk.LEFT, padx=10)
tk.Button(button_frame, text="🔄 RESET", font=("Arial", 14), bg="red", fg="white",
          command=app.reset).pack(side=tk.LEFT, padx=10)

def on_closing():
    app.running = False
    if app.ser:
        app.ser.close()
    root.destroy()

root.protocol("WM_DELETE_WINDOW", on_closing)
root.mainloop()
