/*
 * 'Application code' header file.
 * Contains high-level logic for actions like re-drawing the
 * display and processing input events.
 */
#ifndef __VVC_APP_H
#define __VVC_APP_H

// Include global application values and definitions.
#include "global.h"

// Re-draw the framebuffer to match the current application state.
void redraw_fb( void );

// Process new button input(s).
void process_buttons( void );

#endif
