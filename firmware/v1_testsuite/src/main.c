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
    // Clear the display to the currently-selected background color.
    cur_color = rgb565( bg_r, bg_g, bg_b );
    ufb_fill_rect( &framebuffer, cur_color, 0, 0, 240, 320 );

    // Print the current 'GPS receive' ringbuffer.
    ufb_draw_lines( &framebuffer, ( cur_color ^ 0xFFFF ), 8, 0, ( char* )gps_rb.buf, 1, UFB_ORIENT_H );

    // Delay briefly.
    delay_cycles( 5000000 );

    // Debug: toggle the 'heartbeat' LED.
    //gpio_toggle( GPIOA, 15 );
  }
}
