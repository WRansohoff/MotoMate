# TFT Backlight Test

This project tests the TFT display's brightness control. There's a single NPN transistor holding the backlight anode and cathode apart, with the base pin connected to a PWM channel. Higher PWM duty cycles mean a brighter backlight.

To test that functionality, this project enables the appropriate PWM channel and pulses its duty cycle back and forth; if it works, the display should dim and brighten in pulses.
