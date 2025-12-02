# AWS-IoT-ESP32-Environment-Monitor
"A secure, real-time IoT monitoring system using ESP32, DHT11 sensor, and AWS IoT Core via MQTT over TLS."

# 🌐 AWS IoT Secure Environmental Monitor

An IoT system powered by ESP32 that streams real-time environmental data (Temperature & Humidity) to **AWS IoT Core** securely using **MQTT over TLS**.

## 🚀 Features
- **Secure Communication:** Uses X.509 Certificates (mTLS) for authentication with AWS.
- **Real-Time Data:** Streams DHT11 sensor data every 2 seconds via MQTT.
- **Robustness:** Includes auto-reconnect logic for Wi-Fi and Cloud disruptions.
- **Feedback:** Local LED indicators for physical button interactions.

## 🛠️ Tech Stack
- **Hardware:** ESP32 Dev Module 

[Image of Arduino IDE interface]
, DHT11 Sensor
- **Cloud:** AWS IoT Core
- **Protocol:** MQTT (Port 8883)
- **Library:** ArduinoJson, MQTT (Joel Gaehwiler)

## ⚙️ How to Run
1. Clone this repository.
2. Open `src/main.ino` in Arduino IDE.
3. Create a file named `secrets.h` inside the `src` folder.
4. Add your Wi-Fi credentials and AWS Certificates in `secrets.h` (See code structure).
5. Upload to ESP32.

## 🔒 Security Note
The `secrets.h` file containing Private Keys and Wi-Fi passwords is excluded from this repository for security reasons.

---
*Project created as part of IoT Cloud learning path.*
