# ESP8266 FreeRTOS Development Environment

Docker-based ESP8266 FreeRTOS SDK development environment for Ubuntu.

## Directory Structure
esp8266/
├── scripts/                    # Shared utility scripts
│   ├── extract_sdk.sh         # Extract SDK headers for IntelliSense
│   └── new_project.sh         # Create new project from template
├── _project_template/         # Template for new projects
├── .sdk_headers/              # Extracted SDK headers (not in git)
└── [your_projects]/           # Your ESP8266 projects

## Initial Setup

### 1. Install Docker

Follow instructions at: https://docs.docker.com/engine/install/ubuntu/

### 2. Build Docker Image
```bash
git clone https://github.com/brinth/ESP8266_RTOS_SDK_Docker.git
cd ESP8266_RTOS_SDK_Docker
docker build -t esp8266-rtos .
