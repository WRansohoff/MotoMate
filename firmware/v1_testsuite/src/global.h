/*
 * Header file to hold global definitions.
 */
#ifndef __VVC_GLOBAL_H
#define __VVC_GLOBAL_H

// Preprocessor macros for menu items.
// Application modes.
#define MODE_MAIN_MENU  ( 0 ) /* 'Main menu' */
#define MODE_GPS_RX     ( 1 ) /* 'View messages from GPS module' */
#define MODE_AUDIO      ( 2 ) /* 'Test speaker and audio amp' */
// Menu selections.
#define SEL_MAIN_GPS_RX ( 0 ) /* 'View messages from GPS module' */
#define SEL_MAIN_AUDIO  ( 1 ) /* 'Test speaker and audio amp' */

// Length of ringbuffer to hold data received from the GPS module.
#define GPS_RINGBUF_LEN ( 1024 )

#endif
