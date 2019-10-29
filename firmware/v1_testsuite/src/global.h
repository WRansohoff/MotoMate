/*
 * Header file to hold global declarations.
 */
#ifndef __VVC_GLOBAL_H
#define __VVC_GLOBAL_H

// Standard library includes.
#include <stdint.h>
#include <stdlib.h>

// Utility includes.
#include "ili9341.h"
#include "ringbuf.h"
#include "ufb.h"

// Preprocessor macros for menu items.
// Application modes.
#define MODE_MAIN_MENU ( 0 ) /* 'Main menu' */
#define MODE_GPS_RX    ( 1 ) /* 'View messages from GPS module' */
#define MODE_AUDIO     ( 2 ) /* 'Test speaker and audio amp' */
#define MODE_BACKLIGHT ( 3 ) /* 'Set display brightness' */
#define MODE_BATTERY   ( 4 ) /* 'Check battery level' */
#define MODE_SD_CARD   ( 5 ) /* 'Test microSD connection' (TODO) */
#define MODE_USB       ( 6 ) /* 'Test USB connection' (TODO) */
// Menu selections.
// Main menu:
#define SEL_MAIN_GPS_RX    ( 0 )
#define SEL_MAIN_AUDIO     ( 1 )
#define SEL_MAIN_BACKLIGHT ( 2 )
#define SEL_MAIN_BATTERY   ( 3 )
#define SEL_MAIN_SD_CARD   ( 4 )
#define SEL_MAIN_USB       ( 5 )

// Preprocessor macros for button IDs.
#define BTN_NONE        ( 0 )
#define BTN_UP          ( 3 )
#define BTN_DOWN        ( 4 )
#define BTN_RIGHT       ( 5 )
#define BTN_LEFT        ( 6 )
#define BTN_CENTER      ( 7 )
#define BTN_MODE        ( 8 )
#define BTN_BACK        ( 14 )

// Maximum / minimum number of samples for an audio sine wave.
#define MIN_SINE_SAMPLES ( 16 )
#define MAX_SINE_SAMPLES ( 256 )

// Length of ringbuffer to hold data received from the GPS module.
#define GPS_RINGBUF_LEN ( 1024 )

/* Extern declarations for values defined in the source file. */
// Audio buffer and values.
extern volatile int amp_div, cur_samples, should_play, cur_hz;
extern volatile uint16_t SINE_WAVE[ MAX_SINE_SAMPLES ];
// Display framebuffer and values.
extern volatile uint16_t FRAMEBUFFER[ ILI9341_A ];
extern uFB framebuffer;
extern volatile float tft_brightness;
// GPS module ringbuffer and values.
extern char gps_rb_buf[ GPS_RINGBUF_LEN + 1 ];
extern ringbuf gps_rb;
// Application values.
// Current background color values.
extern volatile uint16_t bg_r, bg_g, bg_b;
// Current application 'mode' and menu selection.
extern volatile int cur_mode, cur_selection;
// Values to track whether certain events have occurred.
extern volatile int new_button_press, new_gps_messages;
// Current system clock speed, in Hz.
extern uint32_t SystemCoreClock;

#endif
