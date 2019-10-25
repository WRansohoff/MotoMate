# Battery Check

This project verifies that the board can read its own battery voltage. It initializes the display and the appropriate ADC channel, then it reads the channel and prints the raw ADC value to the display.

# TODO

I need to add a 'draw float' method to my simple framebuffer library, so that the example can draw an estimated voltage instead of the raw ADC value.

`VBatt = ( ADC * 2 / 4095 ) * VReg`

Where `VBatt` is the battery voltage and `VReg` is the regulated supply voltage. `VReg` is probably 3.3V, but sometimes I use 3V.
