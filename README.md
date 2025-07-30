# 🩺 Medibox – Smart IoT-Controlled Medicine Box

Medibox is a smart, IoT-powered medicine storage system built using the ESP32 microcontroller. It monitors **temperature** and **light intensity** to ensure optimal storage conditions, and uses a **servo motor** to automate access control. Real-time telemetry is visualized via **Node-RED** dashboards using **MQTT**, with adjustable parameters like sampling interval and ideal temperature.

This project is fully simulated using **Wokwi**, making it easy to test and demonstrate without any physical hardware.

---

## 🚀 Features

* 🌡️ Temperature Monitoring using DHT22 sensor
* 💡 Light Intensity Tracking via LDR sensor
* 🔄 Servo Motor Control for smart medicine access
* 📊 Real-Time Dashboards via Node-RED UI (gauges + charts)
* 📤 MQTT Communication using public HiveMQ broker
* ⚙️ Dynamic controls: sampling/sending intervals, ideal temperature, etc.
* 🧪 Fully Simulated on [Wokwi](https://wokwi.com) (no hardware required)

---

## 📁 Repository Structure

| File/Folder           | Description                                         |
| --------------------- | --------------------------------------------------- |
| `sketch.ino`          | Main ESP32 code to handle sensors, servo, and MQTT  |
| `wokwi-project.txt`   | Project ID for Wokwi simulation                     |
| `diagram.json`        | Wokwi circuit diagram (ESP32 + DHT22 + LDR + Servo) |
| `flows.json`          | Node-RED flow for UI and MQTT topics                |
| `libraries.txt`       | Required libraries (for Wokwi or Arduino IDE)       |
| `Code Demo Video.mp4` | Video demonstration of Medibox in action            |

---

## 🧪 Simulate in Wokwi

This project can be simulated entirely in your browser using Wokwi:

🔗 **[👉 Launch Simulation on Wokwi](https://wokwi.com/projects/431125389593782273)**&#x20;

Or paste this Project ID into [https://wokwi.com](https://wokwi.com):

```
431125389593782273
```

Wokwi automatically loads:

* Hardware layout (`diagram.json`)
* Arduino code (`sketch.ino`)
* Required libraries (`libraries.txt`)

---

## 🌐 Node-RED Setup

### 🛠️ Installing Node-RED

To use the real-time dashboard UI (gauges, charts, and sliders), you'll need **Node-RED** installed locally or on a server.

📖 Official Installation Docs:
👉 [https://nodered.org/docs/getting-started/](https://nodered.org/docs/getting-started/)

### Quick Local Installation (using Node.js):

```bash
# Install Node.js if not already installed
https://nodejs.org

# Then install Node-RED
npm install -g --unsafe-perm node-red

# Run Node-RED
node-red
```

Access the flow editor at:
👉 `http://localhost:1880`

### 🚦 Importing the Dashboard

1. Open Node-RED (at `http://localhost:1880`)
2. Click the menu (top-right) → *Import*
3. Paste or upload the contents of `flows.json`
4. Deploy the flow

Ensure MQTT broker is set to:

* **Host:** `broker.hivemq.com`
* **Port:** `1883`

This enables real-time updates for:

* 🌡️ Temperature Gauge + Chart
* 💡 Light Intensity Gauge + Chart
* 🔁 Servo Motor Angle Monitor
* 🎛️ Sliders to control servo behavior and data flow

---

## 📦 Arduino Library Requirements

Install these via Arduino Library Manager or auto-imported in Wokwi:

* `DHT sensor library for ESPx`
* `PubSubClient`
* `ESP32Servo`

---

## 📽️ Demo

Watch the `Code Demo Video.mp4` to see the full workflow and results.

---

## 📜 License

Licensed under the MIT License. Feel free to fork, adapt, and build upon it!


