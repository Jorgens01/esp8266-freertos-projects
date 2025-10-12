#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <project_name>"
    echo "Example: $0 blink_led"
    exit 1
fi

PROJECT_NAME=$1
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASE_DIR="$(dirname "$SCRIPT_DIR")"
TEMPLATE_DIR="$BASE_DIR/_project_template"
NEW_PROJECT_DIR="$BASE_DIR/$PROJECT_NAME"

if [ -d "$NEW_PROJECT_DIR" ]; then
    echo "Error: Project '$PROJECT_NAME' already exists!"
    exit 1
fi

echo "Creating new project: $PROJECT_NAME"

# Copy template
cp -r "$TEMPLATE_DIR" "$NEW_PROJECT_DIR"

# Update Makefile with project name
sed -i "s/PROJECT_NAME := my_project/PROJECT_NAME := $PROJECT_NAME/" "$NEW_PROJECT_DIR/Makefile"

# Update README
sed -i "s/ESP8266 Project Name/ESP8266 $PROJECT_NAME/" "$NEW_PROJECT_DIR/README.md"

echo ""
echo "✓ Project '$PROJECT_NAME' created successfully!"
echo ""
echo "Next steps:"
echo "  cd $NEW_PROJECT_DIR"
echo "  ../scripts/build.sh .        # Build from root"
echo "  # OR"
echo "  cd $NEW_PROJECT_DIR && code .  # Open in VSCode and press Ctrl+Shift+B"
echo ""