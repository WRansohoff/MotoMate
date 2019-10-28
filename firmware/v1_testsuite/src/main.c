// Standard library includes.
#include <stdint.h>
#include <stdlib.h>

// Vendor-provided device header file.
#include "stm32l4xx.h"

// Utility includes.
#include "ili9341.h"
#include "ringbuf.h"
#include "ufb.h"

// Global definitions.
#include "global.h"

// BSP include.
#include "hal/motomate_v1.h"

// 320x240-pixel 16-bit (RGB-565) framebuffer.
// Note: it's 150KB of RAM.
volatile uint16_t FRAMEBUFFER[ ILI9341_A ];
uFB framebuffer = {
  w: ILI9341_W,
  h: ILI9341_H,
  buf: ( uint16_t* )&FRAMEBUFFER
};
// Float to store the desired display brightness (between [0.0, 1.0])
volatile float tft_brightness = 0.67;

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

/**
 * Main program.
 */
int main(void) {
  // Call the high-level 'board_init' method to set up pins and
  // peripherals. The method can be found in the BSP header file,
  // currently `../../fw_lib/bsp/hal/motomate_v1.h`
  board_init();

  // Main application loop.
  uint16_t cur_color;
  while (1) {
    // Wait for an ongoing display DMA transfer operation to complete
    // before modifying the framebuffer.
    while ( ( DMA1_Channel3->CCR & DMA_CCR_EN ) ) { __WFI(); }

    // Clear the display to the currently-selected background color.
    cur_color = rgb565( bg_r, bg_g, bg_b );
    ufb_fill_rect( &framebuffer, cur_color, 0, 0, 240, 320 );

    if ( cur_mode == MODE_MAIN_MENU ) {
      // Draw text for each menu item.
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ), 12, 16, "Test raw NMEA GPS Messages", 1, UFB_ORIENT_H );
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ), 22, 16, "Test speaker audio", 1, UFB_ORIENT_H );
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ), 32, 16, "Set display brightness", 1, UFB_ORIENT_H );
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ), 42, 16, "Check battery voltage", 1, UFB_ORIENT_H );
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ), 52, 16, "Test microSD card (NOT IMPLEMENTED)", 1, UFB_ORIENT_H );
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ), 62, 16, "Test USB connectivity (NOT IMPLEMENTED)", 1, UFB_ORIENT_H );
      // Draw a triangle next to the current selection.
      int tri_base = 6;
      if ( cur_selection == SEL_MAIN_AUDIO ) { tri_base = 16; }
      else if ( cur_selection == SEL_MAIN_BACKLIGHT ) { tri_base = 26; }
      else if ( cur_selection == SEL_MAIN_BATTERY ) { tri_base = 36; }
      else if ( cur_selection == SEL_MAIN_SD_CARD ) { tri_base = 46; }
      else if ( cur_selection == SEL_MAIN_USB ) { tri_base = 56; }
      ufb_fill_rect( &framebuffer, ( cur_color ^ 0xFFFF ), tri_base, 4, 6, 2 );
      ufb_fill_rect( &framebuffer, ( cur_color ^ 0xFFFF ), tri_base + 1, 6, 4, 2 );
      ufb_fill_rect( &framebuffer, ( cur_color ^ 0xFFFF ), tri_base + 2, 8, 2, 2 );
    }
    else if ( cur_mode == MODE_GPS_RX ) {
      // Print the current 'GPS receive' ringbuffer.
      ufb_draw_lines( &framebuffer, ( cur_color ^ 0xFFFF ), 8, 0, ( char* )gps_rb.buf, 1, UFB_ORIENT_H );
    }
    else if ( cur_mode == MODE_AUDIO ) {
      // TODO
    }
    else if ( cur_mode == MODE_BACKLIGHT ) {
      // TODO
    }
    else if ( cur_mode == MODE_BATTERY ) {
      // TODO
    }
    else if ( cur_mode == MODE_SD_CARD ) {
      // TODO
    }
    else if ( cur_mode == MODE_USB ) {
      // TODO
    }

    // Re-trigger a DMA transfer to draw to the display.
    DMA1_Channel3->CCR |=  ( DMA_CCR_EN );

    if ( cur_mode == MODE_MAIN_MENU || cur_mode == MODE_AUDIO ) {
      // Wait for a button press before continuing.
      while ( new_button_press == BTN_NONE ) { __WFI(); }
      // TODO: It would be better to process button inputs here,
      // instead of in the interrupt handlers which should be short.
      new_button_press = BTN_NONE;
    }
    else if ( cur_mode == MODE_GPS_RX ) {
      // Wait for a button press or new GPS messages before continuing.
      while ( new_button_press == BTN_NONE &&
              !new_gps_messages ) {
        __WFI();
      }
      new_button_press = BTN_NONE;
      new_gps_messages = 0;
    }
  }
}
