#!/bin/bash

EXTRACT_DIR="$HOME/my_portfolio/ESP32/esp8266/.sdk_headers"

echo "Extracting ESP8266 RTOS SDK headers to $EXTRACT_DIR..."

# Create the directory
mkdir -p "$EXTRACT_DIR"

# Copy headers from Docker container
docker run -it --rm \
    -v "$EXTRACT_DIR":/sdk_output \
    esp8266-rtos \
    bash -c "cp -r /opt/esp/ESP8266_RTOS_SDK/components /sdk_output/"

echo "Headers extracted successfully to: $EXTRACT_DIR/components"
echo ""
echo "Verifying extraction..."
if [ -f "$EXTRACT_DIR/components/freertos/include/freertos/FreeRTOS.h" ]; then
    echo "✓ FreeRTOS.h found"
else
    echo "✗ FreeRTOS.h NOT found"
fi

if [ -f "$EXTRACT_DIR/components/esp8266/include/esp_system.h" ]; then
    echo "✓ esp_system.h found"
else
    echo "✗ esp_system.h NOT found"
fi
