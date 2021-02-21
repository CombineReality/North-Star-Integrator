

The Integrator is our custom-built USB hub system originally created for the CombineReality Project North Star Deck X. The Integrator cuts down the use of cables and adds customizable buttons to the headset with the following components & features:

    USB-C hub, two USB 3.1 ribbon connectors, and one USB 2.0 ribbon connector
    3GB on-board flash drive (only works when connected to a USB 3.0 host)
    Arduino-compatible microcontroller, featuring a Qwiic connector that can be used to connect additional sensors like an IMU, as well as HID buttons that can emulate keyboard    keys. A button breakout board is included, and the microcontroller is preflashed with firmware that maps the buttons to the default ergonomics adjustment keys. (Eye relief, eye position, and IPD)
    Also allows for manual power reset of sensor USB ports via a GPIO pin.
    A fan, the speed of which is controlled by the Arduino-compatible microcontroller.
    A thermistor for a more intelligent fan speed control
    A ribbon connector that lets the Arduino on the hub relay commands and debug output to and from the serial UART of the display driver board
    Ribbon adapter board for Intel® RealSense™ T261 embedded 6-DOF module (ribbon cable included)
    Ribbon adapter board for Leap Motion Controller (ribbon cable included)
    
Uses a modified version of the lilyPadUSB-caterina Arduino bootloader
Bootloader can be found here: https://github.com/CombineReality/ArduinoCore-avr
