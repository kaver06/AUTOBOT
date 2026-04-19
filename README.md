# 🤖 Autobot – Autonomous Campus Delivery Robot  

Autobot is an autonomous ground robot designed for campus delivery and navigation tasks.  
It combines sensor fusion, motor control, and modular design to perform navigation, obstacle avoidance, and docking in real-world environments.  

---

## 🚀 Project Overview  

**Objective:** Develop a fully autonomous robot that can navigate campus environments, avoid obstacles, and follow pre-defined paths while carrying small payloads.  

**Key Features:**  
- AprilTag-based localization (ESP32 AprilTag + pose estimation library)  
- IMU & encoder-based odometry  
- Line following & obstacle avoidance modes  
- ESP32–STM32 coordination for sensing & control  
- Self-docking and charging capability  
- RTOS-based task scheduling with state machines  
- Mobile app integration with Firebase backend for real-time user interaction  

---

## 🛠️ Hardware  

**Microcontrollers:**  
- STM32F446ZE (motor control, HAL drivers, RTOS)  
- ESP32 (AprilTag detection, pose estimation, Wi-Fi/BLE communication)  

**Motor Driver:**  
- IBT-2 (BTS7960 H-bridge)  

**Locomotion:**  
- Differential drive (2 DC motors)  

**Sensors:**  
- IMU (orientation)  
- Wheel encoders (odometry)  
- IR / ultrasonic (obstacle detection)  
- Camera (ESP32-based AprilTag detection & pose estimation)  

**Power:**  
- Battery pack  
- Self-docking charging station  

---

## ⚙️ Software & Tools  

- STM32CubeIDE + HAL library (low-level control)  
- ESP-IDF (ESP32 programming + AprilTag library integration)  
- FreeRTOS (state machine task scheduling)  
- Firebase (cloud database + real-time sync with mobile app)  
- Mobile App (Flutter / Android) for user-side interaction and delivery management  
- ROS 2 (future) for higher-level data handling & visualization  

---

## 📂 Project Structure
### 🔹 Core System Versions
#### AutoBot v1.0
- Hardware calibration + sensor data acquisition  
- Interrupt & timer-based control (bare-metal)  
- Line following and path execution using direct sensor logic  
#### AutoBot v1.1
- Full upgrade of v1.0 with FreeRTOS integration  
- Task-based architecture (motor control, sensing, communication)  
- State machine-driven execution for scalable autonomy  

---

## 📌 Current Status  

- ✅ Full FreeRTOS integration across STM32 modules (task-based architecture with state machines)
- ✅ Modular code structure implemented (motor_control, sensing, communication)
- ✅ Custom Python-based GUI developed for real-time monitoring, debugging, and data logging
- 🔄 AprilTag-based localization in progress (ESP32 + pose estimation)
- 🔄 Firebase integration with mobile app prototype complete


---

## 🎯 Future Plans  

- Tight ROS 2 integration with Raspberry Pi as high-level controller
- Navigation stack (path planning, mapping, visualization)
- ROS 2 nodes for sensor fusion, control commands, and telemetry
- Integrate Python GUI with ROS 2 ecosystem
- Subscribe to ROS 2 topics for real-time visualization
- Logging and playback of robot telemetry (rosbag or custom logger)
- Debug interface for testing individual modules (motor, sensors, localization)
- STM32 ↔ ROS 2 bridge (via UART/Wi-Fi using ESP32 or direct serial interface)
- Autonomous navigation with dynamic obstacle handling (ROS 2 decision layer + STM32 execution layer)
- Cloud logging of delivery paths and performance (Firebase + ROS 2 data streams)
- Mobile app with Firebase for user-side delivery requests, robot status, and notifications
- Fully optimized self-docking using sensor fusion + ROS 2 feedback loop
- Expandable payload system with task-based delivery scheduling

---

## 👤 Team  

- [Kaver S A](https://github.com/kaver06)  
- [Anagha N G](https://github.com/Anagha-ng)  
