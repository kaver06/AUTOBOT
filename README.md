🤖 Autobot – Autonomous Campus Delivery Robot
Autobot is an autonomous ground robot designed for campus delivery and navigation tasks.
It combines sensor fusion, motor control, and modular design to perform navigation, obstacle avoidance, and docking in real-world environments.
________________________________________
🚀 Project Overview
Objective: Develop a fully autonomous robot that can navigate campus environments, avoid obstacles, and follow pre-defined paths while carrying small payloads.
Key Features:
•	AprilTag-based localization (ESP32 AprilTag + pose estimation library)
•	IMU & encoder-based odometry
•	Line following & obstacle avoidance modes
•	ESP32–STM32 coordination for sensing & control
•	Self-docking and charging capability
•	RTOS-based task scheduling with state machines
•	Mobile app integration with Firebase backend for real-time user interaction
________________________________________
🛠️ Hardware
•	Microcontrollers:
  o	STM32F446ZE (motor control, HAL drivers, RTOS)
  o	ESP32 (AprilTag detection, pose estimation, Wi-Fi/BLE communication)
•	Motor Driver: IBT-2 (BTS7960 H-bridge)
•	Locomotion: Differential drive (2 DC motors)
•	Sensors:
  o	IMU (orientation)
  o	Wheel encoders (odometry)
  o	IR / ultrasonic (obstacle detection)
  o	Camera (ESP32-based AprilTag detection & pose estimation)
•	Power:
  o	Battery pack
  o	Self-docking charging station
________________________________________
⚙️ Software & Tools
•	STM32CubeIDE + HAL library (low-level control)
•	ESP-IDF (ESP32 programming + AprilTag library integration)
•	FreeRTOS (state machine task scheduling)
•	Firebase (cloud database + real-time sync with mobile app)
•	Mobile App (Flutter / Android) for user-side interaction and delivery management
•	ROS 2 (future) for higher-level data handling & visualization
________________________________________
📂 Project Modules
•	motor_control/ → Low-level motor drivers (HAL, STM32)
•	line_follower/ → Line following algorithm (IR sensors)
•	obstacle_avoidance/ → Ultrasonic/IR-based avoidance logic
•	localization/ → ESP32 AprilTag detection + IMU + encoder fusion
•	docking/ → Charging station docking logic
•	communication/ → ESP32 ↔ STM32 bridge
•	firebase_integration/ → Firebase database + mobile app sync for task allocation and status updates
________________________________________
📌 Current Status
•	✅ Motor driver tested (forward, backward, turns)
•	✅ Basic modular code structure set up
•	🔄 Integrating line follower and obstacle avoidance
•	🔄 Testing ESP32–STM32 communication
•	🔄 AprilTag-based localization in progress (ESP32 library)
•	🔄 RTOS task manager and state machine design in progress
•	🔄 Firebase integration with mobile app prototype in progress
________________________________________
🎯 Future Plans
•	Full RTOS implementation across all modules
•	Autonomous navigation with dynamic obstacle handling
•	Cloud logging of delivery paths and performance
•	Mobile app with Firebase for user-side delivery requests, robot status, and notifications
•	Expandable payload system
________________________________________
👤 Team
•	GitHub – Kaver S A
•	GitHub – Anagha N G

