# Opening Bridge Control System — Software Documentation Overview (Team KK)

##  Project Overview
This repository contains the **software subsystem** for the ENGG2000/3000 SPINE Engineering Project — a **sensor-driven, remotely controllable opening bridge**, developed by **Team KK**.  
The system integrates **ESP32 microcontrollers**, motor drivers, ultrasonic sensors, LEDs, and a wireless user interface to manage bridge operations for both pedestrian and vehicular traffic, while allowing ships to pass safely.

This repository focuses on the **SYSTEMS** group deliverables, which include:
- Embedded firmware for the ESP32
- Ultrasonic sensor integration for ship detection
- Motor control logic for opening/closing the bridge
- LED-based traffic signalling
- Remote user interface via Wi-Fi
- Communication between hardware subsystems and UI

---

##  Team KK Members
| Name | Student ID | Email |
|------|------------|-------|
| Kyle Chow | 46586091 | kyle.chow@students.mq.edu.au |
| Ryan Norin | 46999795 | ryan.norin@students.mq.edu.au |
| Lemar Khatiz | 47288744 | lemar.khatiz@students.mq.edu.au |
| Chas Liddell-Vassallo | 48582441 | chas.liddellvassallo@students.mq.edu.au |
| Aarya Nepal | 47401427 | aarya.nepal1@students.mq.edu.au |
| Jared Kaw | 46632166 | jared.kaw@students.mq.edu.au |

---

##  Bill of Materials
| Component | Qty | Use | Price (AUD) |
|-----------|-----|-----|-------------|
| ESP32 Development Board ([link](https://mikroelectron.com/product/esp32-development-board-type-c-usb-ch340c)) | 1 | Main microcontroller | $7.00 |
| L298N Motor Driver | 1 | Bridge motor control | $7.00 |
| 12V 251RPM 18kg.cm DC Motor | 1 | Bridge lifting mechanism | $45.00 |
| 12V → 5V Step-Down Converter | 1 | Power regulation | $2.55 |
| Multi-colour LEDs | 4 | Traffic and status indicators | $1.95 ea |
| HC-SR04 Ultrasonic Sensors | 4 | Ship detection | $1.95 ea |

## Functional Requirements
**Bridge Operation Logic**
- Detect ship arrival via ultrasonic sensors.
- Stop pedestrian/vehicular traffic with LED signals.
- Raise bridge to ≥200mm clearance.
- After passage, lower bridge to closed position (≥50mm above river).
- Resume pedestrian/vehicular traffic.

**Traffic Signal Control**
- LED-based Red/Green/Yellow lights for vehicle and pedestrian traffic.
- Ship Wait/Pass visual indication.

**User Interface (UI)**
- Graphical web-based dashboard.
- Live status display of:
  - Bridge position
  - Traffic signals
  - Sensor readings
- Manual override to:
  - Open/close bridge
  - Simulate sensor inputs
  - Start/stop system

---

##  Non-Functional Requirements
- **Connectivity:** Wi-Fi communication between ESP32 and UI.
- **Power:** All electronics run from a single 12V 5A supply.
- **Cost:** Total BOM ≤ $100 (excluding base plate).
- **Concealment:** All wiring and electronics hidden.

---

##  System Features
- **Automatic Mode:** Fully autonomous detection and bridge operation.
- **Manual Override:** Full remote control.
- **Visual Feedback:** Local LEDs for system state.
- **Safety Interlocks:** Prevents bridge motion when unsafe.

---

##  Communication & Interfaces
**Inputs:**
- Ultrasonic sensors for ship detection.
- UI commands via Wi-Fi.

**Outputs:**
- Motor control signals to L298N driver.
- LED traffic lights.
- UI status updates.

**Protocols:**
- HTTP/WebSocket (Wi-Fi) for UI ↔ ESP32.
- Digital I/O for sensors/motor driver.

---

##  Testing & Validation
1. **Unit Testing:** Motor control, sensor inputs, LED signals, UI commands.
2. **Integration Testing:** ESP32 ↔ Motors ↔ Sensors.
3. **System Testing:** Full bridge operation under load and clearance requirements.
