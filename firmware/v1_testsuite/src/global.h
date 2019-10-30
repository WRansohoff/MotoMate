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

/** Current system clock speed, in Hz. */
extern uint32_t SystemCoreClock;

// Audio buffer values.
/** Audio test option: Amplitude division. Lower values mean higher volume. */
extern volatile int amp_div;
/** Audio test option: Sine wave resolution. Larger values mean more samples in the sine wave, which should sound better? */
extern volatile int cur_samples;
/** Audio test option: 'Mute' option, to enable or disable audio output in software. */
extern volatile int should_play;
/** Audio test option: Test tone target frequency, in Hertz. */
extern volatile int cur_hz;
/** Minimum allowable sine wave resolution. */
#define MIN_SINE_SAMPLES ( 16 )
/** Maximum allowable sine wave resolution. */
#define MAX_SINE_SAMPLES ( 256 )
/** Buffer to hold a sine wave which, when sent to the audio amplifier through a DAC at the right timing, will produce an audio tone. */
extern volatile uint16_t SINE_WAVE[ MAX_SINE_SAMPLES ];

// Values to track whether certain events have occurred.
/** Maximum number of button presses to queue for processing. */
#define MAX_BTN_PRESSES ( 8 )
/** Buffer to store button presses until they can be processed. */
extern volatile int new_button_presses[ MAX_BTN_PRESSES ];
/** This flag is set to a non-zero value when a new set of messages has been received from the GPS module. */
extern volatile int new_gps_messages;

// GPS module ringbuffer and values.
/** Length of the ringbuffer which holds data received from the GPS module. */
#define GPS_RINGBUF_LEN ( 1024 )
/** Static storage for a ringbuffer which holds messages from the GPS modules. */
extern char gps_rb_buf[ GPS_RINGBUF_LEN + 1 ];
/** Ringbuffer to store messages from the GPS module while they wait to be processed. */
extern ringbuf gps_rb;

// Display framebuffer and values.
/** Static storage for the display's framebuffer. This uses 150KB of RAM to store a 240x320-pixel display with 16bpp of color. */
extern volatile uint16_t FRAMEBUFFER[ ILI9341_A ];
/** Main framebuffer object for the display. */
extern uFB framebuffer;
/** Desired brightness for the display's backlight, on a scale from [0.0 : 1.0] */
extern volatile float tft_brightness;

/** Current background color: red value. */
extern volatile uint16_t bg_r;
/** Current background color: green value. */
extern volatile uint16_t bg_g;
/** Current background color: blue value. */
extern volatile uint16_t bg_b;

// Current application 'mode' and menu selection.
/** This variable stores the current application 'mode'. */
extern volatile int cur_mode;
/** This variable stores the currently-selected menu item within the current application 'mode'. */
extern volatile int cur_selection;

// Preprocessor macros for button IDs.
/** Button ID: No button pressed. */
#define BTN_NONE        ( 0 )
/** Button ID: Navigation switch 'down' button. */
#define BTN_DOWN        ( 3 )
/** Button ID: Navigation switch 'up' button. */
#define BTN_UP          ( 4 )
/** Button ID: Navigation switch 'right' button. */
#define BTN_RIGHT       ( 5 )
/** Button ID: Navigation switch 'left' button. */
#define BTN_LEFT        ( 6 )
/** Button ID: Navigation switch 'center' button. */
#define BTN_CENTER      ( 7 )
/** Button ID: 'mode' button. */
#define BTN_MODE        ( 8 )
/** Button ID: 'back' button. */
#define BTN_BACK        ( 14 )

/*
 * Application modes: these are flags representing
 * different 'states' that the program can be in.
 */
/** 'Main Menu' mode flag. */
#define MODE_MAIN_MENU ( 0 )
/** 'View messages from GPS module' mode flag. */
#define MODE_GPS_RX    ( 1 )
/** 'Test speaker and audio amp' mode flag. */
#define MODE_AUDIO     ( 2 )
/** 'Set display brightness' mode flag. */
#define MODE_BACKLIGHT ( 3 )
/** 'Check battery level' mode flag. */
#define MODE_BATTERY   ( 4 )
/** 'Test touch screen' mode flag. (TODO: Not implemented) */
#define MODE_TOUCH     ( 5 )
/** 'Test microSD connection' mode flag. (TODO: Not implemented) */
#define MODE_SD_CARD   ( 6 )
/** 'Test USB connection' mode flag. (TODO: Not implemented) */
#define MODE_USB       ( 7 )

/*
 * Menu selections: these are flags representing
 * different options which can be selected in various menus.
 */
// Main menu:
/** Main menu selection flag: 'View messages from GPS module' */
#define SEL_MAIN_GPS_RX    ( 0 )
/** Main menu selection flag: 'Test speaker and audio amp' */
#define SEL_MAIN_AUDIO     ( 1 )
/** Main menu selection flag: 'Set display brightness' */
#define SEL_MAIN_BACKLIGHT ( 2 )
/** Main menu selection flag: 'Check battery level' */
#define SEL_MAIN_BATTERY   ( 3 )
/** Main menu selection flag: 'Test touch screen' (TODO: Not implemented) */
#define SEL_MAIN_TOUCH     ( 4 )
/** Main menu selection flag: 'Test microSD connection' (TODO: Not implemented) */
#define SEL_MAIN_SD_CARD   ( 5 )
/** Main menu selection flag: 'Test USB connection' (TODO: Not implemented) */
#define SEL_MAIN_USB       ( 6 )

// Audio menu:
/** Audio menu selection flag: 'Mute' option. */
#define SEL_AUDIO_MUTE     ( 0 )
/** Audio menu selection flag: 'Audio tone frequency' option. */
#define SEL_AUDIO_FREQ     ( 1 )
/** Audio menu selection flag: 'Sine wave resolution' option. (# of samples) */
#define SEL_AUDIO_SAMPLES  ( 2 )
/** Audio menu selection flag: 'Amplitude division' option. (volume control) */
#define SEL_AUDIO_AMPDIV   ( 3 )

#endif
