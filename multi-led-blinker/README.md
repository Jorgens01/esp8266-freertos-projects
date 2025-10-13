# Multi-LED Blinker (FreeRTOS Concurrency Demo)

A demonstration project for the **ESP8266 FreeRTOS SDK** showcasing true concurrency by managing two independent GPIO tasks. This is the foundational method for managing non-blocking, periodic events alongside critical components like the Wi-Fi stack.

## 💡 Key Concepts Demonstrated

* **FreeRTOS Task Separation:** Using two separate tasks (`led_blinker_task_1` and `led_blinker_task_2`) to achieve independent timing control.
* **Non-Blocking Delay:** Employing **`vTaskDelay()`** to yield CPU time, which is essential for preventing **Watchdog Timer (WDT) Timeouts** on the ESP8266.
* **Safe GPIO Usage:** Utilizing general-purpose GPIO pins (4 and 5) that do not interfere with the ESP8266's boot strapping modes.

## ⏱️ Timing Details

| LED Task | GPIO Pin | Blink Period (High/Low) | Frequency |
| :--- | :--- | :--- | :--- |
| **LED 1** | **GPIO4** | 500 ms | 2 Hz |
| **LED 2** | **GPIO5** | 1000 ms | 1 Hz |

## 🔌 Hardware Requirements

* **ESP8266** module (e.g., NodeMCU, ESP-01S, ESP-12F)
* **2 x LEDs**
* **2 x Current-Limiting Resistors** (typically 220 $\Omega$ to 1 k$\Omega$)

### Wiring

Connect the components as follows:

1.  **LED 1:** Connect one lead of a resistor to **GPIO4**. Connect the other resistor lead to the **Anode** (long leg) of the LED. Connect the **Cathode** (short leg) to **GND**.
2.  **LED 2:** Connect one lead of a resistor to **GPIO5**. Connect the other resistor lead to the **Anode** of the second LED. Connect the **Cathode** to **GND**.

## 📝 Usage

This project supports execution from both its directory (`multi_led_blinker/`) and the repository root, leveraging your shared build scripts.

### Execution from Project Directory

To run commands from within the project directory, use the relative path to the shared scripts:

```bash
# Build the project inside the Docker environment
../scripts/build.sh .

# Flash the resulting binary to the ESP8266 and open the serial monitor
# This is a convenience script that runs flash.sh and then monitor.sh
../scripts/flash_monitor.sh .
```

### Execution from Root Directory
To manage the project directly from the repository root, pass the project directory name as the first argument to the scripts:

```bash
# Build the project from the root
./scripts/build.sh multi_led_blinker

# Flash and monitor the project from the root
./scripts/flash_monitor.sh multi_led_blinker
```

### Configuration (Optional)
To review or change FreeRTOS or SDK settings specific to this project (e.g., stack sizes, clock frequency):
```bash
# From project directory
../scripts/menuconfig.sh .
```
```bash
# From root directory
./scripts/menuconfig.sh multi_led_blinker
```