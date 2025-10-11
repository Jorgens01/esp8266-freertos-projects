#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# Change this to your actual device
DEVICE="${ESP8266_DEVICE:-/dev/ttyUSB0}"

docker run -it --rm \
    --privileged \
    -v "$PROJECT_DIR":/project \
    -v $DEVICE:/dev/ttyUSB0 \
    -w /project \
    esp8266-rtos \
    make flash
