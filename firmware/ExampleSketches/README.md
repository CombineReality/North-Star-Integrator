`Disclaimer: Use these altered sketches at your own risk, I don't take any responsibility for any damage caused to hardware as a result of using them. Thermistor temperature reading code was attained through ChatGPT`

# Example sketches for North Star Integrator

- `NorthStarButtons` is the original sketch from CombineReality.
- `NorthStarButtonsPWM` is the original sketch with added thermistor/PWM fan curve.
- `NorthStarBUttonsPWM170` adds the thermistor/PWM fan curve but removes `digitalWrite(PORT_1, LOW)` so that the SR 170 is not power cycled in the `powerReset()` function. I was having issues with my SR 170 being recognised on USB reset which is why I implemented this.

## Hardware
According to the [schematic](https://github.com/CombineReality/North-Star-Integrator/raw/master/NS_Integrator_schematic.pdf), and [original BOM ](https://www.smart-prototyping.com/image/data/NOA-RnD/North%20star%20Deck%20X/Smart-Prototyping.com%20Deck%20X%20V2020-08-20%20-%20Public%20BOM.pdf)the related hardware is as follows: 
- Fan `Mini-Fan with custom 5cm cable`
- Thermistor: `MF52B 103F3950-100mm` [(link)](https://www.lcsc.com/product-detail/NTC-Thermistors_Nanjing-Shiheng-Elec-MF52B-103F3950-100_C394021.html)
  - 10KΩ NTC Thermistor set on `line 75`
  - 3950K beta value set on `line 77`
  - Resistor of 4.7KΩ for the voltage divider [(source)](https://raw.githubusercontent.com/CombineReality/North-Star-Integrator/master/docs/arduino_pins.png) set on `line 81`
- I have bluetacked the thermistor to the centre front section of the T261 as it seems to run hottest.

## Changing the PWM curve

- Set your room temperature in Kelvin on `line 79`
- After testing, the I found lowest PWM value `(between 0-255)` you can feasibly set is `55`. Anything lower than this and the fan doesn't have enough power to spin.
- `int getFanSpeed(int temperature)` implicitly converts the temperature reading from the function `float getTemp()` to an integer at the start of the `void loop()` function. It uses this value to calculate the PWM value.
- By default, the sketches clamp the fan PWM to `55` for temperatures lower than 35°C, and `255` for temperatures over 70°C. It uses the `map()` function to linearly interpolate the PWM to `55-160` between these two temperatures (the default fan PWM from CombineReality was 160). Change these values as you wish.

## Uploading sketches to the North Star

1. On Linux, you may have to add yourself to the usergroup that `/dev/ttyACM0` (the Arduino Leonardo's serial port) is part of if you are receiving errors uploading sketches.
   1. Run `stat /dev/ttyACM0` in a terminal
   2. Note the user group that the serial port is assigned to, e.g. `dialout` or `plugdev`
   3. Add your user to this group with `sudo usermod  -a -G [group] [username]` 
      1. `sudo` run command as superuser 
      2. `usermod` modify user attributes
      3.  `-a` append 
      4.  `-G` group

2. Add the `../libraries/SX1508` folder to your Arduino library directory (default on Linux: `/home/[user]/Arduino/libraries`)
   1. `../libraries/SX1508/src/util/sx1508_registers.h` has been renamed to `SX1508_registers.h` in this repo because I was getting errors when I tried to access this file in the Arduino IDE on Linux, I believe this was because of [UNIX case-sensitivity](https://en.wikipedia.org/wiki/Case_sensitivity).
3. Open a sketch in Arduino IDE
4. Install any additionally required libraries (on Linux I needed to add the `Keyboard` library)
5. Set the correct Port and Board in `Tools > `
6. Verify and Upload the sketch.