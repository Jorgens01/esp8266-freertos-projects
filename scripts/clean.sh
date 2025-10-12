#!/bin/bash

PROJECT_DIR="${1:-.}"
PROJECT_DIR="$(cd "$PROJECT_DIR" && pwd)"

echo "Cleaning project: $PROJECT_DIR"

docker run -it --rm \
    -v "$PROJECT_DIR":/project \
    -w /project \
    esp8266-rtos \
    make clean