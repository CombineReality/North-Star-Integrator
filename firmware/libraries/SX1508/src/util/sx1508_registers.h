/******************************************************************************
sx1509_registers.h
Register definitions for SX1509.
Jim Lindblom @ SparkFun Electronics
Original Creation Date: September 21, 2015
https://github.com/sparkfun/SparkFun_SX1509rduino_Library

Here you'll find the Arduino code used to interface with the SX1509 I2C
16 I/O expander. There are functions to take advantage of everything the
SX1509 provides - input/output setting, writing pins high/low, reading 
the input value of pins, LED driver utilities (blink, breath, pwm), and
keypad engine utilites.

Development environment specifics:
	IDE: Arduino 1.6.5
	Hardware Platform: Arduino Uno
	SX1509 Breakout Version: v2.0

This code is beerware; if you see me (or any other SparkFun employee) at the
local, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.
******************************************************************************/

#define 	REG_INPUT_DISABLE		0x00	//	RegInputDisableA Input buffer disable register _ I/O[7_0] (Bank A) 0000 0000
#define 	REG_LONG_SLEW			0x01	//	RegLongSlewA Output buffer long slew register _ I/O[7_0] (Bank A) 0000 0000
#define 	REG_LOW_DRIVE			0x02	//	RegLowDriveA Output buffer low drive register _ I/O[7_0] (Bank A) 0000 0000
#define 	REG_PULL_UP				0x03	//	RegPullUpA Pull_up register _ I/O[7_0] (Bank A) 0000 0000
#define 	REG_PULL_DOWN			0x04	//	RegPullDownA Pull_down register _ I/O[7_0] (Bank A) 0000 0000
#define 	REG_OPEN_DRAIN			0x05	//	RegOpenDrainA Open drain register _ I/O[7_0] (Bank A) 0000 0000
#define 	REG_POLARITY			0x06	//	RegPolarityA Polarity register _ I/O[7_0] (Bank A) 0000 0000
#define 	REG_DIR					0x07	//	RegDirA Direction register _ I/O[7_0] (Bank A) 1111 1111
#define 	REG_DATA				0x08	//	RegDataA Data register _ I/O[7_0] (Bank A) 1111 1111*
#define 	REG_INTERRUPT_MASK		0x09	//	RegInterruptMaskA Interrupt mask register _ I/O[7_0] (Bank A) 1111 1111
#define 	REG_SENSE_HIGH			0x0A	//	RegSenseHighA Sense register for I/O[7:4] 0000 0000
#define 	REG_SENSE_LOW			0x0B	//	RegSenseLowA Sense register for I/O[3:0] 0000 0000
#define 	REG_INTERRUPT_SOURCE	0x0C	//	RegInterruptSourceA Interrupt source register _ I/O[7_0] (Bank A) 0000 0000
#define 	REG_EVENT_STATUS		0x0D	//	RegEventStatusA Event status register _ I/O[7_0] (Bank A) 0000 0000
#define 	REG_LEVEL_SHIFTER		0x0E	//	RegLevelShifter1 Level shifter register 0000 0000
#define 	REG_CLOCK				0x0F	//	RegClock Clock management register 0000 0000
#define 	REG_MISC				0x10	//	RegMisc Miscellaneous device settings register 0000 0000
#define 	REG_LED_DRIVER_ENABLE	0x11	//	RegLEDDriverEnableA LED driver enable register _ I/O[7_0] (Bank A) 0000 0000
// Debounce and Keypad Engine		
#define 	REG_DEBOUNCE_CONFIG		0x12	//	RegDebounceConfig Debounce configuration register 0000 0000
#define 	REG_DEBOUNCE_ENABLE		0x13	//	RegDebounceEnableA Debounce enable register _ I/O[7_0] (Bank A) 0000 0000
#define 	REG_KEY_CONFIG			0x14	//	RegKeyConfig1 Key scan configuration register 0000 0000
#define 	REG_KEY_DATA			0x15	//	RegKeyData1 Key value (column) 1111 1111
// LED Driver (PWM, blinking, breathing)		
#define 	REG_I_ON_0				0x16	//	RegTOn0 ON time register for I/O[0] 0000 0000
#define 	REG_I_ON_1				0x17	//	RegIOn1 ON intensity register for I/O[1] 1111 1111
#define 	REG_T_ON_2				0x18	//	RegTOn2 ON time register for I/O[2] 0000 0000
#define 	REG_I_ON_2				0x19	//	RegIOn2 ON intensity register for I/O[2] 1111 1111
#define 	REG_OFF_2				0x1A	//	RegOff2 OFF time/intensity register for I/O[2] 0000 0000
#define 	REG_T_ON_3				0x1B	//	RegTOn3 ON time register for I/O[3] 0000 0000
#define 	REG_I_ON_3				0x1C	//	RegIOn3 ON intensity register for I/O[3] 1111 1111
#define 	REG_OFF_3				0x1D	//	RegOff3 OFF time/intensity register for I/O[3] 0000 0000
#define 	REG_T_RISE_3			0x1E	//	RegTRise4 Fade in register for I/O[4] 0000 0000
#define 	REG_T_FALL_3			0x1F	//	RegTFall4 Fade out register for I/O[4] 0000 0000
#define 	REG_I_ON_4				0x20	//	RegIOn4 ON intensity register for I/O[4] 1111 1111
#define 	REG_I_ON_5				0x21	//	RegIOn5 ON intensity register for I/O[5] 1111 1111
#define 	REG_T_ON_6				0x22	//	RegTOn6 ON time register for I/O[6] 0000 0000
#define 	REG_I_ON_6				0x23	//	RegIOn6 ON intensity register for I/O[6] 1111 1111
#define 	REG_OFF_6				0x24	//	RegOff6 OFF time/intensity register for I/O[6] 0000 0000
#define 	REG_T_ON_7				0x25	//	RegTOn7 ON time register for I/O[7] 0000 0000
#define 	REG_I_ON_7				0x26	//	RegIOn7 ON intensity register for I/O[7] 1111 1111
#define 	REG_OFF_7				0x27	//	RegOff7 OFF time/intensity register for I/O[7] 0000 0000
#define 	REG_T_RISE_7			0x28	//	RegTRise7 Fade in register for I/O[7] 0000 0000
#define 	REG_T_FALL_7			0x29	//	RegTFall7 Fade out register for I/O[7] 0000 0000

// 	Miscellaneous		
#define 	REG_HIGH_INPUT		0x2A	//	RegHighInputA High input enable register _ I/O[7_0] (Bank A) 0000 0000
//  Software Reset		
#define 	REG_RESET				0x7D	//	RegReset Software reset register 0000 0000
#define 	REG_TEST_1				0x7E	//	RegTest1 Test register 0000 0000
#define 	REG_TEST_2				0x7F	//	RegTest2 Test register 0000 0000

byte REG_I_ON[8] = {REG_I_ON_0, REG_I_ON_1, REG_I_ON_2, REG_I_ON_3,
					REG_I_ON_4, REG_I_ON_5, REG_I_ON_6, REG_I_ON_7};
					
byte REG_T_ON[8] = { 0xFF, 0xFF, REG_T_ON_2, REG_T_ON_3,
					0xFF, 0xFF, REG_T_ON_6, REG_T_ON_7};
					
byte REG_OFF[8] = { 0xFF, 0xFF, REG_OFF_2, REG_OFF_3,
					0xFF, 0xFF, REG_OFF_6, REG_OFF_7};

byte REG_T_RISE[8] = {0xFF, 0xFF, 0xFF, 0xFF,
					0xFF, 0xFF, 0xFF, REG_T_RISE_7};
					
byte REG_T_FALL[8] = {0xFF, 0xFF, 0xFF, 0xFF,
					0xFF, 0xFF, 0xFF, REG_T_FALL_7};