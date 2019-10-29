/*
 * Source file to hold global definitions.
 */
#include "global.h"

// Audio buffers and testing values.
volatile int amp_div = 1;
volatile int cur_samples = 32;
volatile int should_play = 0;
volatile int cur_hz = 440;
volatile uint16_t SINE_WAVE[ MAX_SINE_SAMPLES ];

// 320x240-pixel 16-bit (RGB-565) framebuffer.
// Note: it's 150KB of RAM.
volatile uint16_t FRAMEBUFFER[ ILI9341_A ];
uFB framebuffer = {
  w: ILI9341_W,
  h: ILI9341_H,
  buf: ( uint16_t* )&FRAMEBUFFER
};
// Float to store the desired display brightness (between [0.0, 1.0])
volatile float tft_brightness = 0.65;

// Ringbuffer for holding data sent by the GPS module.
char gps_rb_buf[ GPS_RINGBUF_LEN + 1 ];
ringbuf gps_rb = {
  len: GPS_RINGBUF_LEN,
  buf: gps_rb_buf,
  pos: 0,
  ext: 0
};

// Solid color to set the display to; button presses will change this.
volatile uint16_t bg_r = 0x1F;
volatile uint16_t bg_g = 0x00;
volatile uint16_t bg_b = 0x1F;

// Which 'mode' the application is in. Basically, which menu to
// display or which application to run.
volatile int cur_mode = MODE_GPS_RX;
// Current menu selection item.
volatile int cur_selection = SEL_MAIN_GPS_RX;
// Variable to track whether a new button press was received.
// TODO: Use a queue?
volatile int new_button_press = BTN_NONE;
// Variable to track whether a new set of GPS messages are ready.
volatile int new_gps_messages = 0;

// Global variable to hold the core clock speed in Hertz.
uint32_t SystemCoreClock = 4000000;
