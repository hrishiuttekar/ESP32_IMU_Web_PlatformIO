<div align="center">

# 🚀 ESP32 MPU6500 IMU Web Dashboard

**Stream live gyroscope, accelerometer & temperature data from an MPU6500 to your browser, and watch a 3D rocket move with your sensor in real time.**

![ESP32](https://img.shields.io/badge/ESP32-DevKit-E7352C?style=for-the-badge&logo=espressif&logoColor=white)
![PlatformIO](https://img.shields.io/badge/PlatformIO-Arduino-F5822A?style=for-the-badge&logo=platformio&logoColor=white)
![Three.js](https://img.shields.io/badge/Three.js-3D-000000?style=for-the-badge&logo=threedotjs&logoColor=white)
![Sensor](https://img.shields.io/badge/IMU-MPU6500%20%7C%20MPU6050-2C7BE5?style=for-the-badge)

</div>

---

## ✨ Features

- 📡 **Real-time streaming** over Server-Sent Events (`/events`), no page refresh needed
- 🌀 **Gyroscope** angles (rad), updated every ~10 ms
- 📈 **Accelerometer** X/Y/Z (m/s²), updated every 200 ms
- 🌡️ **On-chip temperature** (°C), updated every second
- 🚀 **3D rocket model** built in Three.js that rotates with the sensor
- 🎯 **Reset buttons** to zero all axes or just X / Y / Z
- 🔍 **Auto-detects the chip** via `WHO_AM_I` (`0x70` = MPU6500, `0x68` = MPU6050)
- 💾 Web UI served from **LittleFS** flash storage
- 🔐 Wi-Fi credentials kept out of git in `secrets.h`

## 🧰 Hardware

| Part | Notes |
|------|-------|
| ESP32 Dev Board | `esp32dev` (DOIT DevKit V1 or similar) |
| MPU6500 IMU | MPU6050 also works |
| Jumper wires | 4× female-female |

### 🔌 Wiring

| MPU6500 | ESP32 |
|:-------:|:-----:|
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |
| AD0 | GND (I²C address `0x68`) |

## ⚡ Quick Start

### 1. Clone

```bash
git clone https://github.com/hrishiuttekar/ESP32_IMU_Web_PlatformIO.git
cd ESP32_IMU_Web_PlatformIO
```

### 2. Add your Wi-Fi credentials

Copy the template and fill in your network details:

```bash
cp src/secrets.example.h src/secrets.h
```

```cpp
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```

> `src/secrets.h` is in `.gitignore`, so your password never gets committed.

### 3. Upload the web files (LittleFS)

```bash
pio run --target uploadfs
```

### 4. Build & flash the firmware

```bash
pio run --target upload
pio device monitor
```

### 5. Open the dashboard

**Keep the sensor still** while it boots so the gyro can calibrate. The serial monitor then prints the board's IP address:

```
I2C address 0x68, WHO_AM_I = 0x70
Keep the sensor still while the gyro calibrates.
Connecting to WiFi....
192.168.1.42
```

Open `http://<that-ip>/` in any browser on the same network. 🎉

## 🌐 HTTP API

| Endpoint | Method | Description |
|----------|:------:|-------------|
| `/` | GET | Dashboard (`index.html`) |
| `/events` | SSE | Live stream: `gyro_readings`, `accelerometer_readings`, `temperature_reading` |
| `/reset` | GET | Zero all gyro angles |
| `/resetX` · `/resetY` · `/resetZ` | GET | Zero a single axis |

Example `gyro_readings` payload:

```json
{ "gyroX": "0.12", "gyroY": "-0.40", "gyroZ": "1.05", "accX": "0.21", "accY": "-0.08", "accZ": "9.79" }
```

## 📁 Project Structure

```
ESP32_IMU_Web_PlatformIO/
├── data/                    # Web UI (uploaded to LittleFS)
│   ├── index.html
│   ├── script.js            # SSE client + Three.js rocket
│   └── style.css
├── src/
│   ├── main.cpp             # Firmware: Wi-Fi, IMU, web server
│   ├── secrets.example.h    # Wi-Fi template → copy to secrets.h
│   └── JarzebskiMPU6050/    # Bundled MPU6050/6500 driver
└── platformio.ini
```

## 🛠️ Troubleshooting

| Problem | Fix |
|---------|-----|
| `Could not read the IMU` | Check SDA/SCL wiring and 3V3 power. AD0 must be on GND for `0x68` |
| `LittleFS files are missing` | Run `pio run --target uploadfs` |
| Rocket drifts slowly | Press **RESET POSITION**, and keep the sensor still during boot calibration |
| Stuck on `Connecting to WiFi...` | Check `secrets.h`. The ESP32 only supports **2.4 GHz** networks |
| `secrets.h: No such file` | You skipped step 2. Copy `secrets.example.h` to `secrets.h` |

## 📦 Dependencies

Installed automatically by PlatformIO:

- [ESP32Async/AsyncTCP](https://github.com/ESP32Async/AsyncTCP) `3.5.0`
- [ESP32Async/ESPAsyncWebServer](https://github.com/ESP32Async/ESPAsyncWebServer) `3.11.2`
- [arduino-libraries/Arduino_JSON](https://github.com/arduino-libraries/Arduino_JSON) `0.2.2`
- [Three.js r107](https://threejs.org/) (loaded from CDN in the browser)

## 🙏 Credits & License

- Modified for the **MPU6500** by **Hrishi Uttekar**: chip auto-detection, a new 3D rocket model, and the PlatformIO setup.
- Based on the [ESP32 MPU-6050 Web Server](https://RandomNerdTutorials.com/esp32-mpu-6050-web-server/) tutorial by **Rui Santos & Sara Santos** (Random Nerd Tutorials), used under its permission notice:
  > Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files. The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
- The MPU6050 driver in `src/JarzebskiMPU6050/` is by **Korneliusz Jarzębski** and is licensed under **GPL-3.0** (see its [LICENSE](src/JarzebskiMPU6050/LICENSE)).

<div align="center">

⭐ **If this project helped you, give it a star!** ⭐

</div>
