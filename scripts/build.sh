#!/bin/bash

# If called with a project path argument, use it
# Otherwise, use current directory
PROJECT_DIR="${1:-.}"
PROJECT_DIR="$(cd "$PROJECT_DIR" && pwd)"

echo "Building project: $PROJECT_DIR"

docker run -it --rm \
    -v "$PROJECT_DIR":/project \
    -w /project \
    esp8266-rtos \
    make
