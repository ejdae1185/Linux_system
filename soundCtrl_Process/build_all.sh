#!/bin/bash

# Build launcher module
echo "Building launcher module..."
cmake --build launcherModule/build
if [ $? -ne 0 ]; then
    echo "Failed to build launcher module"
    exit 1
fi

# Build input module
echo "Building input module..."
cmake --build inputModule/build
if [ $? -ne 0 ]; then
    echo "Failed to build input module"
    exit 1
fi

# Build output module
echo "Building output module..."
cmake --build outputModule/build

if [ $? -ne 0 ]; then
    echo "Failed to build output module"
    exit 1
fi

# Run input module
echo "Building viewer module..."
cmake --build viewerModule/build

if [ $? -ne 0 ]; then
    echo "Failed to build viewer module"
    exit 1
fi

# Run output module
echo "Building modulation module..."
cmake --build modulationModule/build/

if [ $? -ne 0 ]; then
    echo "Failed to build modulation module"
    exit 1
fi