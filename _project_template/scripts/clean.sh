#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

docker run -it --rm \
    -v "$PROJECT_DIR":/project \
    -w /project \
    esp8266-rtos \
    make clean
