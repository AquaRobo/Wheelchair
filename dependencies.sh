#!/bin/bash
set -e

echo "Updating system packages..."
sudo apt update

echo "Installing dependencies..."
sudo apt install -y python3-pip
sudo apt install -y ros-humble-rosbridge-server
sudo snap install mjpg-streamer

echo "Installing Python dependencies..."
pip3 install --upgrade pip
pip3 install \
    pyshine \
    opencv-python \
    adafruit-blinka \
    adafruit-io \
    spidev

echo "Installation complete!"
