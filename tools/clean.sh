#!/bin/bash

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "Cleaning project..."

# Remove object files
find "$PROJECT_ROOT" -name "*.o" -type f -delete

# Remove Python cache
find "$PROJECT_ROOT" -name "__pycache__" -type d -exec rm -rf {} +
find "$PROJECT_ROOT" -name "*.pyc" -type f -delete

# Remove build artifacts
rm -rf "$PROJECT_ROOT/build_modules"

# Remove node_modules
echo "Removing node_modules..."
find "$PROJECT_ROOT" -name "node_modules" -type d -prune -exec rm -rf {} +

# Remove executables and VSIX
[ -f "$PROJECT_ROOT/prometheus" ] && rm "$PROJECT_ROOT/prometheus"
find "$PROJECT_ROOT" -name "*.vsix" -type f -delete

echo "Cleanup complete."