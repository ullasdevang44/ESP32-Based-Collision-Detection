# ESP32 Collision Detection and Smart Vehicle Alert System

## Overview
This project demonstrates a smart vehicle system using **ESP32**, capable of detecting collisions in real-time and sending **email alerts with location data**.  
It also supports **manual vehicle control via Bluetooth** and provides visual/audio alerts using LEDs and a buzzer.  

The system is suitable for **safety monitoring**, **IoT vehicle projects**, and **smart robotics experiments**.

---

## Features
- Real-time collision detection using a digital sensor.  
- Automatic stop of motors on collision.  
- Email alert sent to multiple recipients with location information.  
- Bluetooth control for manual driving: Forward, Backward, Left, Right, Stop.  
- Visual and audio indicators via LEDs and buzzer.  
- Wi-Fi based location fetching via IP API.

---

## Hardware Requirements

| Component | Quantity | Notes |
|-----------|---------|-------|
| ESP32 Development Board | 1 | Main controller |
| Collision Sensor | 1 | Detects collisions |
| DC Motors | 2 | Actuators for movement |
| Motor Driver (L298N or H-Bridge) | 1 | Motor control |
| LEDs (Red/Green) | 2 | Front/Back indicators |
| Buzzer | 1 | Audible alert |
| Jumper Wires | 1 set | Connections |
| Li-ion Battery Pack | 1 | Power supply |

---

## Software Requirements
- **Arduino IDE** (latest version)  
- Libraries:  
  - `BluetoothSerial`  
  - `WiFiManager`  
  - `HTTPClient`  
  - `Arduino_JSON`  
  - `ESP_Mail_Client`  
- Internet connection for sending email alerts.

---

## Pin Allocation

| IN/OUT/PWR | PIN NAME | Purpose |
|------------|----------|---------|
| IN | GPIO34 | Collision Sensor Input |
| OUT | GPIO27 | Motor IN1 |
| OUT | GPIO26 | Motor IN2 |
| OUT | GPIO25 | Motor IN3 |
| OUT | GPIO33 | Motor IN4 |
| OUT/PWM | GPIO32 | Motor EN (Speed Control) |
| OUT | GPIO12 | Front LED |
| OUT | GPIO13 | Back LED |
| OUT | GPIO15 | Buzzer |
| PWR | 3.3V / 5V | Power |
| PWR | GND | Ground |


---

## Usage

### Setup
1. Connect all hardware according to the wiring diagram.  
2. Ensure ESP32 is powered and connected to Wi-Fi.

### Bluetooth Control
- Connect to the ESP32 via Bluetooth (`ESP32_Car`).  
- Send commands:  
  - `F` → Forward  
  - `B` → Backward  
  - `L` → Left  
  - `R` → Right  
  - `S` → Stop  

### Collision Detection
- On impact, the collision sensor triggers:  
  - Motors stop immediately.  
  - LEDs and buzzer turn on for visual/audio alert.  
  - Email alert is sent with the current location.

### Email Setup
- Update `AUTHOR_EMAIL`, `AUTHOR_PASSWORD`, and recipient emails in the code.  
- Make sure Gmail “less secure app access” is enabled or use an app password.

### Location
- Location is fetched via IP-based API (`http://ip-api.com/json/`).  
- Google Maps link is included in the email.

---

## Bill of Materials (BOM)

| Category | Component | Quantity | Unit Price (₹) | Total Price (₹) |
|----------|-----------|---------|----------------|----------------|
| Controller | ESP32 | 1 | 600 | 600 |
| Sensor | Collision Sensor | 1 | 150 | 150 |
| Motor Driver | L298N | 1 | 250 | 250 |
| Actuators | DC Motors | 2 | 200 | 400 |
| Indicators | LEDs | 2 | 10 | 20 |
| Indicators | Buzzer | 1 | 30 | 30 |
| Connectivity | Jumper Wires | 1 set | 100 | 100 |
| Power | Li-ion Battery | 1 | 500 | 500 |
| Software | Arduino IDE / Libraries | - | Free | - |
| **Total** |  |  |  | **2050** |

---

## Code
- Main code is located in the [`Code/`](Code/) folder.  
- Optional test sketches or calibration scripts are in [`Code/calibration_examples/`](Code/calibration_examples/).

---

## Future Scope
- Integrate **GPS module** for precise location.  
- Add **SMS or WhatsApp notifications**.  
- Connect to **IoT Dashboard** (Blynk/Firebase) for live monitoring.  
- Implement **accident severity detection** using accelerometer data.  
- Add **automatic emergency calls or voice alerts**.

---

## License
This project is **open-source**. You can freely use, modify, and distribute it for **educational or non-commercial purposes**.
