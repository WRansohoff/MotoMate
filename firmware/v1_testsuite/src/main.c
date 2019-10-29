// Vendor-provided device header file.
#include "stm32l4xx.h"

// Global definitions.
#include "global.h"

// BSP include.
#include "hal/motomate_v1.h"

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

    // Re-draw the framebuffer.
    redraw_fb();

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
