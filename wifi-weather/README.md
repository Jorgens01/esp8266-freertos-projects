# ESP8266 Wi-Fi Weather Station ☁️

## 📝 Brief Description

This project utilizes the **ESP8266** microcontroller running the **FreeRTOS-SDK** to connect to a Wi-Fi network (Station mode) and establish a stable foundation for a weather application. It features robust Wi-Fi synchronization using FreeRTOS Event Groups and includes a system monitor task for logging connection status and real-time Free Heap memory size.

The next planned steps involve integrating a REST API client (HTTP) to fetch weather data from OpenWeatherMap, followed by JSON parsing.

---

## 🛠️ Hardware Requirements

* **ESP8266 Module** (e.g., ESP-01, ESP-12E/F, NodeMCU).
* **USB-to-Serial adapter** (or integrated USB port, e.g., on NodeMCU boards).
* **Power supply** capable of providing stable 500mA+ (especially during Wi-Fi operation).

---

## ⚙️ Setup and Configuration

This project uses shell scripts located in the `scripts/` directory to simplify the standard ESP-IDF commands. These scripts are designed to be executed from the **project's root directory**.

### 1. Connect and Path Check

1.  Connect your ESP8266 to your computer via the serial adapter.
2.  **Verify Device Path:** If your serial port is not `/dev/ttyUSB0` (the default for Linux/macOS), you **must** update the device path (`ESPPORT`) inside your scripts (e.g., `build.sh`, `flash_monitor.sh`) before proceeding. Common paths include `COM3` on Windows or `/dev/ttyACM0` on some Linux systems.

### 2. Configure the Project

Run the configuration script to open the `menuconfig` tool:

```bash
../scripts/menuconfig.sh
```

**Crucial step:** Navigate to the custom configuration options and set your Wi-Fi credentials:
- `CONFIG_WIFI_SSID`
- `CONFIG_WIFI_PASSWORD`
Save the configuration and exit `menuconfig`.

## ⚙️ Build, Flash, and Monitor
Use the provided scripts to handle the build and flashing process seamlessly.

| Action | Command | Description |
| :--- | :--- | :--- |
| **Build** | `../scripts/build.sh` | Compiles the project source code. |
| **Flash & Monitor** | `../scripts/flash_monitor.sh` | Flashes

**Expected Output**

Upon running the monitor, you should observe the NVS initialization, the Wi-Fi connection attempt, followed by the IP address acquisition. The `connection_monitor_task` will then periodically report a stable connection status and the current free heap size.

