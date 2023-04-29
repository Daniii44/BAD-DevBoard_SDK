# Hardware Errata
## USB
- USB-C Port Pinout flipped horizontally
- ESP32 USB Datalines switch up
- Resistors on both CC lines instead of just on one

## LEDs
- ATTiny LED Resistor should be bigger than the others to account for the higher GPIO voltage
- Driving the LEDs with 2mA instead of 20mA is easily sufficient and avoids the otherwise notable temperature increase 