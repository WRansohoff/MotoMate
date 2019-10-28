/*
 * Header file to hold global definitions.
 */
#ifndef __VVC_GLOBAL_H
#define __VVC_GLOBAL_H

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


// Length of ringbuffer to hold data received from the GPS module.
#define GPS_RINGBUF_LEN ( 1024 )

#endif
