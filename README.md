# ESP8266 FreeRTOS Projects

![License](https://img.shields.io/badge/License-MIT-yellow.svg)
![ESP8266](https://img.shields.io/badge/ESP8266-FreeRTOS-blue)
![Docker](https://img.shields.io/badge/Docker-Enabled-2496ED?logo=docker&logoColor=white)

A collection of ESP8266 embedded systems projects using FreeRTOS SDK with a Docker-based development environment.

## 🚀 Projects

- **[hello_world](./hello_world/)** - Basic FreeRTOS application with chip info and task delays

## 📁 Structure
esp8266-freertos-projects/
├── scripts/              # Shared build scripts
├── _project_template/    # Template for new projects
├── .sdk_headers/         # SDK headers (not in git)
└── [projects]/           # Individual projects

## 🛠️ Setup

### Prerequisites

- Docker
- Git
- Ubuntu/Linux (with serial port access)

### Initial Setup
```bash
# Clone repository
git clone git@github.com:Jorgens01/esp8266-freertos-projects.git
cd esp8266-freertos-projects
```

```bash
# Clone and build Docker image
git clone https://github.com/brinth/ESP8266_RTOS_SDK_Docker.git
cd ESP8266_RTOS_SDK_Docker
docker build -t esp8266-rtos .
cd ..
```

```bash
# Extract SDK headers for IntelliSense
./scripts/extract_sdk.sh
```

```bash
# Set serial port permissions
sudo usermod -a -G dialout $USER
# Log out and log back in
```

## 📝 Usage
### Create New Project
```bash
./scripts/new_project.sh my_project
cd my_project
../scripts/build.sh .
```

## Build and Flash
```bash
# From project directory
cd hello_world
../scripts/build.sh .
../scripts/flash.sh .
../scripts/monitor.sh .
```

```bash
# From root directory
./scripts/build.sh hello_world
./scripts/flash_monitor.sh hello_world
```

### VSCode
```bash
cd hello_world
code .
# Press Ctrl+Shift+B to build
```

## ⚙️ Configuration
### Change Serial Device
```bash
export ESP8266_DEVICE=/dev/ttyUSB0
```

### Project Settings
```bash
cd hello_world
../scripts/menuconfig.sh .
```

## 🐛 Troubleshooting
### Permission denied on serial port
```bash
sudo usermod -a -G dialout $USER
# Log out and log back in
```

### IntelliSense not working
```bash
./scripts/extract_sdk.sh
cd hello_world && ../scripts/build.sh .
# Reload VSCode
```

### Find serial device
```bash
ls -l /dev/ttyUSB*
dmesg | grep tty
```

## 🔗 Resources
* [ESP8266 RTOS SDK Docs](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/index.html)
* [FreeRTOS Documentation](https://freertos.org/Documentation/02-Kernel/07-Books-and-manual/01-RTOS_book)
