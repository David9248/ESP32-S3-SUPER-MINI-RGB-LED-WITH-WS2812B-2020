# ESP32-S3-SUPER-MINI-RGB-LED-WITH-WS2812B-2020
A LED bar controller with cool effects, controlled via WiFi using ESP32-S3. 

What This Does
Control addressable LED strips from your phone or computer. Plug in your ESP32-S3, upload this code, and you've got a wireless lighting system with cool effects.

What You Need
ESP32-S3 board (the tiny one with USB-C)

WS2812B LED strip (the kind where each LED can be a different color)

Computer with Arduino IDE installed

WiFi network that your devices can connect to

Quick Setup
1. Wiring It Up
text
ESP32-S3     →   LED Strip
─────────────────────────────
5V pin       →   Red wire (5V)
GND pin      →   White wire (GND)
GPIO4 pin    →   Green wire (DATA)
