/*
 * 'Application code' source file.
 * Contains high-level logic for actions like re-drawing the
 * display and processing input events.
 */
#include "app.h"

// Re-draw the framebuffer to match the current application state.
void redraw_fb( void ) {
    // Clear the display to the currently-selected background color.
    uint16_t cur_color = rgb565( bg_r, bg_g, bg_b );
    ufb_fill_rect( &framebuffer, rgb565( bg_r, bg_g, bg_b ), 0, 0, 240, 320 );

    // Draw whatever needs to be drawn for the current mode.
    if ( cur_mode == MODE_MAIN_MENU ) {
      // Draw text for each menu item.
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ), 12, 16,
        "Test raw NMEA GPS Messages", 1, UFB_ORIENT_H );
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ), 22, 16,
        "Test speaker audio", 1, UFB_ORIENT_H );
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ), 32, 16,
        "Set display brightness", 1, UFB_ORIENT_H );
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ), 42, 16,
        "Check battery voltage", 1, UFB_ORIENT_H );
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ), 52, 16,
        "Test touch screen (NOT IMPLEMENTED)", 1, UFB_ORIENT_H );
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ), 62, 16,
        "Test microSD card (NOT IMPLEMENTED)", 1, UFB_ORIENT_H );
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ), 72, 16,
        "Test USB connectivity (NOT IMPLEMENTED)", 1, UFB_ORIENT_H );
      // Draw a triangle next to the current selection.
      int tri_base = 6;
      if ( cur_selection == SEL_MAIN_AUDIO ) { tri_base = 16; }
      else if ( cur_selection == SEL_MAIN_BACKLIGHT ) { tri_base = 26; }
      else if ( cur_selection == SEL_MAIN_BATTERY ) { tri_base = 36; }
      else if ( cur_selection == SEL_MAIN_TOUCH ) { tri_base = 46; }
      else if ( cur_selection == SEL_MAIN_SD_CARD ) { tri_base = 56; }
      else if ( cur_selection == SEL_MAIN_USB ) { tri_base = 66; }
      ufb_fill_rect( &framebuffer, ( cur_color ^ 0xFFFF ),
        tri_base, 4, 6, 2 );
      ufb_fill_rect( &framebuffer, ( cur_color ^ 0xFFFF ),
        tri_base + 1, 6, 4, 2 );
      ufb_fill_rect( &framebuffer, ( cur_color ^ 0xFFFF ),
        tri_base + 2, 8, 2, 2 );
    }
    else if ( cur_mode == MODE_GPS_RX ) {
      // Print the current 'GPS receive' ringbuffer.
      ufb_draw_lines( &framebuffer, ( cur_color ^ 0xFFFF ),
        8, 0, ( char* )gps_rb.buf, 1, UFB_ORIENT_H );
    }
    else if ( cur_mode == MODE_AUDIO ) {
      // (TODO: Track & highlight current menu item.)
      // Draw test title.
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ),
        232, 16, "Audio Test Tone:", 3, UFB_ORIENT_H );
      // Draw explanatory strings.
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ),
        140, 8, "Amplitude Division:", 2, UFB_ORIENT_H );
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ),
        100, 8, "Sine Wave Samples:", 2, UFB_ORIENT_H );
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ),
        60, 8, "Audio Freq. (Hz):", 2, UFB_ORIENT_H );
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ),
        20, 8, "Mute?", 2, UFB_ORIENT_H );
      // Draw boxes outlining the current values of the options.
      ufb_draw_rect( &framebuffer, ( cur_color ^ 0xFFFF ),
        118, 244, 28, 60, 1 );
      ufb_draw_rect( &framebuffer, ( cur_color ^ 0xFFFF ),
        78, 244, 28, 60, 1 );
      ufb_draw_rect( &framebuffer, ( cur_color ^ 0xFFFF ),
        38, 244, 28, 60, 1 );
      // Draw the current values of the options.
      ufb_draw_int( &framebuffer, ( cur_color ^ 0xFFFF ),
        140, 262, amp_div, 2, UFB_ORIENT_H );
      ufb_draw_int( &framebuffer, ( cur_color ^ 0xFFFF ),
        100, 262, cur_samples, 2, UFB_ORIENT_H );
      ufb_draw_int( &framebuffer, ( cur_color ^ 0xFFFF ),
        60, 248, cur_hz, 2, UFB_ORIENT_H );
      // Draw 'left/right' carets.
      ufb_draw_char( &framebuffer, ( cur_color ^ 0xFFFF ),
        136, 238, '<', 1, UFB_ORIENT_H );
      ufb_draw_char( &framebuffer, ( cur_color ^ 0xFFFF ),
        136, 306, '>', 1, UFB_ORIENT_H );
      ufb_draw_char( &framebuffer, ( cur_color ^ 0xFFFF ),
        96, 238, '<', 1, UFB_ORIENT_H );
      ufb_draw_char( &framebuffer, ( cur_color ^ 0xFFFF ),
        96, 306, '>', 1, UFB_ORIENT_H );
      ufb_draw_char( &framebuffer, ( cur_color ^ 0xFFFF ),
        56, 238, '<', 1, UFB_ORIENT_H );
      ufb_draw_char( &framebuffer, ( cur_color ^ 0xFFFF ),
        56, 306, '>', 1, UFB_ORIENT_H );
      // TODO: Draw 'mute?' selector.
    }
    else if ( cur_mode == MODE_BACKLIGHT ) {
      // Draw menu title.
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ),
        232, 46, "Display Brightness:", 2, UFB_ORIENT_H );
      // Draw left/right carets.
      ufb_draw_char( &framebuffer, ( cur_color ^ 0xFFFF ),
        140, 100, '<', 3, UFB_ORIENT_H );
      ufb_draw_char( &framebuffer, ( cur_color ^ 0xFFFF ),
        140, 204, '>', 3, UFB_ORIENT_H );
      // Draw current value.
      int brightness_pct = ( int )( tft_brightness * 100 );
      ufb_draw_int( &framebuffer, ( cur_color ^ 0xFFFF ),
        140, 124, brightness_pct, 3, UFB_ORIENT_H );
      ufb_draw_char( &framebuffer, ( cur_color ^ 0xFFFF ),
        140, 178, '%', 3, UFB_ORIENT_H );
    }
    else if ( cur_mode == MODE_BATTERY ) {
      // Draw test title.
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ),
        232, 16, "Battery Reading:", 3, UFB_ORIENT_H );
      // Draw explanatory strings.
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ),
        180, 8, "ADC Reading:", 2, UFB_ORIENT_H );
      ufb_draw_str( &framebuffer, ( cur_color ^ 0xFFFF ),
        120, 8, "Battery Voltage:", 2, UFB_ORIENT_H );
      // Take an ADC reading, and draw it.
      ufb_draw_int( &framebuffer, ( cur_color ^ 0xFFFF ),
        180, 220, adc_single_conversion( ADC1 ), 2, UFB_ORIENT_H );
      // TODO: Draw converted voltage value.
    }
    else if ( cur_mode == MODE_TOUCH ) {
      // TODO
    }
    else if ( cur_mode == MODE_SD_CARD ) {
      // TODO
    }
    else if ( cur_mode == MODE_USB ) {
      // TODO
    }
}

// Register a new button input.
void register_button_press( int type ) {
  for ( int i = 0; i < MAX_BTN_PRESSES; ++i ) {
    if ( new_button_presses[ i ] == BTN_NONE ) {
      new_button_presses[ i ] = type;
      return;
    }
  }
}

// Process new button input(s).
void process_buttons ( void ) {
  for ( int i = 0; i < MAX_BTN_PRESSES; ++i ) {
    // If there are no more presses to process, done.
    if ( new_button_presses[ i ] == BTN_NONE ) { return; }

    // Process the next button press.
    else if ( new_button_presses[ i ] == BTN_DOWN ) {
      if ( cur_mode == MODE_MAIN_MENU ) {
        // Switch menu selection.
        // Shortcut: decrement by 1 unless it's at the min value.
        if ( cur_selection == SEL_MAIN_GPS_RX ) {
          cur_selection = SEL_MAIN_USB;
        }
        else { --cur_selection; }
      }
      else if ( cur_mode == MODE_GPS_RX ) {
        // Set background the color to purple.
        bg_r = 0x1F;
        bg_g = 0x00;
        bg_b = 0x1F;
      }
      else if ( cur_mode == MODE_AUDIO ) {
        // TODO
      }
      else if ( cur_mode == MODE_BACKLIGHT ) {
        // Decrement brightness and update the PWM signal.
        if ( tft_brightness > 0.05 ) { tft_brightness -= 0.05; }
        timer_pwm_out( TIM3, 4, tft_brightness, 1000000 );
      }
      else if ( cur_mode == MODE_BATTERY ) {
        // TODO
      }
      else if ( cur_mode == MODE_TOUCH ) {
        // TODO
      }
      else if ( cur_mode == MODE_SD_CARD ) {
        // TODO
      }
      else if ( cur_mode == MODE_USB ) {
        // TODO
      }
    }
    else if ( new_button_presses[ i ] == BTN_UP ) {
      if ( cur_mode == MODE_MAIN_MENU ) {
        // Switch menu selection.
        // Shortcut: increment by 1 unless it's at the max value.
        if ( cur_selection == SEL_MAIN_USB ) {
          cur_selection = SEL_MAIN_GPS_RX;
        }
        else { ++cur_selection; }
      }
      else if ( cur_mode == MODE_GPS_RX ) {
        // Set the background color to yellow.
        bg_r = 0x1F;
        bg_g = 0x0C;
        bg_b = 0x00;
      }
      else if ( cur_mode == MODE_AUDIO ) {
        // TODO
      }
      else if ( cur_mode == MODE_BACKLIGHT ) {
        // Increment brightness and update the PWM signal.
        if ( tft_brightness < 0.96 ) { tft_brightness += 0.05; }
        timer_pwm_out( TIM3, 4, tft_brightness, 1000000 );
      }
      else if ( cur_mode == MODE_BATTERY ) {
        // TODO
      }
      else if ( cur_mode == MODE_TOUCH ) {
        // TODO
      }
      else if ( cur_mode == MODE_SD_CARD ) {
        // TODO
      }
      else if ( cur_mode == MODE_USB ) {
        // TODO
      }
    }
    else if ( new_button_presses[ i ] == BTN_RIGHT ) {
      if ( cur_mode == MODE_MAIN_MENU ) {
        // Enter the current menu selection.
        if ( cur_selection == SEL_MAIN_GPS_RX ) {
          cur_mode = MODE_GPS_RX;
        }
        else if ( cur_selection == SEL_MAIN_AUDIO ) {
          cur_mode = MODE_AUDIO;
        }
        else if ( cur_selection == SEL_MAIN_BACKLIGHT ) {
          cur_mode = MODE_BACKLIGHT;
        }
        else if ( cur_selection == SEL_MAIN_BATTERY ) {
          cur_mode = MODE_BATTERY;
        }
        else if ( cur_mode == MODE_TOUCH ) {
          // TODO
        }
        else if ( cur_selection == SEL_MAIN_SD_CARD ) {
          // TODO
          //cur_mode = MODE_SD_CARD;
        }
        else if ( cur_selection == SEL_MAIN_USB ) {
          // TODO
          //cur_mode = MODE_USB;
        }
      }
      else if ( cur_mode == MODE_GPS_RX ) {
        // Set the background color to red.
        bg_r = 0x1F;
        bg_g = 0x00;
        bg_b = 0x00;
      }
      else if ( cur_mode == MODE_AUDIO ) {
        // TODO
      }
      else if ( cur_mode == MODE_BACKLIGHT ) {
        // Increment brightness and update the PWM signal.
        if ( tft_brightness < 0.96 ) { tft_brightness += 0.05; }
        timer_pwm_out( TIM3, 4, tft_brightness, 1000000 );
      }
      else if ( cur_mode == MODE_BATTERY ) {
        // TODO
      }
      else if ( cur_mode == MODE_TOUCH ) {
        // TODO
      }
      else if ( cur_mode == MODE_SD_CARD ) {
        // TODO
      }
      else if ( cur_mode == MODE_USB ) {
        // TODO
      }
    }
    else if ( new_button_presses[ i ] == BTN_LEFT ) {
      if ( cur_mode == MODE_MAIN_MENU ) {
        // Set the background color to green.
        bg_r = 0x00;
        bg_g = 0x2F;
        bg_b = 0x08;
      }
      else if ( cur_mode == MODE_GPS_RX ) {
        // Set the background color to green.
        bg_r = 0x00;
        bg_g = 0x2F;
        bg_b = 0x08;
      }
      else if ( cur_mode == MODE_AUDIO ) {
        // TODO
      }
      else if ( cur_mode == MODE_BACKLIGHT ) {
        // Decrement brightness and update the PWM signal.
        if ( tft_brightness > 0.05 ) { tft_brightness -= 0.05; }
        timer_pwm_out( TIM3, 4, tft_brightness, 1000000 );
      }
      else if ( cur_mode == MODE_BATTERY ) {
        // TODO
      }
      else if ( cur_mode == MODE_TOUCH ) {
        // TODO
      }
      else if ( cur_mode == MODE_SD_CARD ) {
        // TODO
      }
      else if ( cur_mode == MODE_USB ) {
        // TODO
      }
    }
    else if ( new_button_presses[ i ] == BTN_CENTER ) {
      if ( cur_mode == MODE_MAIN_MENU ) {
        // Enter the current menu selection.
        if ( cur_selection == SEL_MAIN_GPS_RX ) {
          cur_mode = MODE_GPS_RX;
        }
        else if ( cur_selection == SEL_MAIN_AUDIO ) {
          cur_mode = MODE_AUDIO;
        }
        else if ( cur_selection == SEL_MAIN_BACKLIGHT ) {
          cur_mode = MODE_BACKLIGHT;
        }
        else if ( cur_selection == SEL_MAIN_BATTERY ) {
          cur_mode = MODE_BATTERY;
        }
        else if ( cur_mode == MODE_TOUCH ) {
          // TODO
        }
        else if ( cur_selection == SEL_MAIN_SD_CARD ) {
          // TODO
          //cur_mode = MODE_SD_CARD;
        }
        else if ( cur_selection == SEL_MAIN_USB ) {
          // TODO
          //cur_mode = MODE_USB;
        }
      }
      else if ( cur_mode == MODE_GPS_RX ) {
        // Set the background color to blue.
        bg_r = 0x00;
        bg_g = 0x00;
        bg_b = 0x1F;
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
      else if ( cur_mode == MODE_TOUCH ) {
        // TODO
      }
      else if ( cur_mode == MODE_SD_CARD ) {
        // TODO
      }
      else if ( cur_mode == MODE_USB ) {
        // TODO
      }
    }
    else if ( new_button_presses[ i ] == BTN_MODE ) {
      // Set the color to teal.
      bg_r = 0x00;
      bg_g = 0x0F;
      bg_b = 0x0F;
    }
    else if ( new_button_presses[ i ] == BTN_BACK ) {
      if ( cur_mode == MODE_GPS_RX ||
           cur_mode == MODE_AUDIO ||
           cur_mode == MODE_BACKLIGHT ||
           cur_mode == MODE_BATTERY ||
           cur_mode == MODE_TOUCH ||
           cur_mode == MODE_SD_CARD ||
           cur_mode == MODE_USB ) {
        // TODO: This relies on a specific ordering of the
        // preprocessor macros, and that's bad practice.
        cur_selection = cur_mode - 1;
        // Return to the main menu and reset selection cursor.
        cur_mode = MODE_MAIN_MENU;
        // Discard any other pending button presses and return.
        for ( int j = 0; j < MAX_BTN_PRESSES; ++j ) {
          new_button_presses[ j ] = BTN_NONE;
        }
        return;
      }
    }

    // Mark the button press as processed.
    new_button_presses[ i ] = BTN_NONE;
  }
}
