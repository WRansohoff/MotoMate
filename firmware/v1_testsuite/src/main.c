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
      // TODO: Draw the currently-selected option in inverted colors,
      // and allow modifying options.
      // TODO: Draw explanatory strings.
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ), 232, 16, "Audio Test Tone:", 3, UFB_ORIENT_H );
      // TODO: Draw boxes outlining the current values of the options.
      // TODO: Draw the current values of the options.
      // TODO: Draw triangles to act as 'left/right' arrows.
    }
    else if ( cur_mode == MODE_BACKLIGHT ) {
      // Draw menu title.
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ), 232, 46, "Display Brightness:", 2, UFB_ORIENT_H );
      // TODO: Draw left/right triangles.
      // Draw current value.
      int brightness_pct = ( int )( tft_brightness * 100 );
      ufb_draw_int( &framebuffer, ( cur_color ^ 0xFFFF ), 140, 124, brightness_pct, 3, UFB_ORIENT_H );
      ufb_draw_char( &framebuffer, ( cur_color ^ 0xFFFF ), 140, 178, '%', 3, UFB_ORIENT_H );
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
