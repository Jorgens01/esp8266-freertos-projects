#!/bin/bash

PROJECT_DIR="${1:-.}"
PROJECT_DIR="$(cd "$PROJECT_DIR" && pwd)"
DEVICE="${ESP8266_DEVICE:-/dev/ttyUSB0}"

echo "Flashing project: $PROJECT_DIR"
echo "Device: $DEVICE"

docker run -it --rm \
    --privileged \
    -v "$PROJECT_DIR":/project \
    -v $DEVICE:/dev/ttyUSB0 \
    -w /project \
    esp8266-rtos \
    make flash