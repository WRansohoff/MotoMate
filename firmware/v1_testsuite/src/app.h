/*
 * 'Application code' header file.
 * Contains high-level logic for actions like re-drawing the
 * display and processing input events.
 */
#ifndef __VVC_APP_H
#define __VVC_APP_H

// HAL includes.
#include "hal/adc.h"
#include "hal/tim.h"

// Include global application values and definitions.
#include "global.h"

// Re-draw the framebuffer to match the current application state.
void redraw_fb( void );

// Register a new button input.
void register_button_press( int type );
// Process new button input(s).
void process_buttons( void );

#endif
