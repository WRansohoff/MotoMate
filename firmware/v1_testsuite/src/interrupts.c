/*
 * (Interrupt handlers go here)
 */

// Standard library includes.
#include <stdint.h>

// Vendor-provided device header file.
#include "stm32l4xx.h"

// HAL includes.
#include "hal/rcc.h"

// BSP includes.
#include "ili9341.h"
#include "ringbuf.h"
#include "ufb.h"

// Global definitions.
#include "global.h"

// DMA1, Channel 3: SPI transmit.
void DMA1_chan3_IRQ_handler( void ) {
  // Since the DMA peripheral can only handle buffers up to 64KWords,
  // we can't sent the whole 75KWord framebuffer at once. So instead,
  // we send one half at a time and update the source address every
  // time that a 'transfer complete' interrupt triggers.
  if ( DMA1->ISR & DMA_ISR_TCIF3 ) {
    // Acknowledge the interrupt.
    DMA1->IFCR |=  ( DMA_IFCR_CTCIF3 );
    // Disable the DMA channel.
    DMA1_Channel3->CCR &= ~( DMA_CCR_EN );
    // Update the 'transfer length' field.
    DMA1_Channel3->CNDTR = ( uint16_t )( ILI9341_A / 2 );
    // Update the source address and refill the 'count' register.
    if ( DMA1_Channel3->CMAR >= ( uint32_t )&( FRAMEBUFFER[ ILI9341_A / 2 ] ) ) {
      DMA1_Channel3->CMAR  = ( uint32_t )&FRAMEBUFFER;
      // Transfer is complete; wait for a trigger to re-draw.
    }
    else {
      DMA1_Channel3->CMAR  = ( uint32_t )&( FRAMEBUFFER[ ILI9341_A / 2 ] );
      // Re-enable the DMA channel to resume the transfer.
      DMA1_Channel3->CCR |=  ( DMA_CCR_EN );
    }
  }
}

// UART4: Communication with GPS transceiver.
void UART4_IRQ_handler( void ) {
  // 'Receive register not empty' interrupt.
  if ( UART4->ISR & USART_ISR_RXNE ) {
    // Copy new data into the ringbuffer.
    char c = UART4->RDR;
    ringbuf_write( gps_rb, c );
  }
  // 'Receive timeout' interrupt.
  if ( UART4->ISR & USART_ISR_RTOF ) {
    // Mark the end of the message in the ringbuffer and reset
    // its position/extent to 0.
    ringbuf_write( gps_rb, '\0' );
    gps_rb.pos = 0;
    gps_rb.ext = 0;
    // Mark that new GPS messages are ready.
    new_gps_messages = 1;
    // Acknowledge the interrupt.
    UART4->ICR |=  ( USART_ICR_RTOCF );
  }
}

// EXTI channel 3: Nav switch 'up'.
void EXTI3_IRQ_handler( void ) {
  if ( EXTI->PR1 & ( 1 << 3 ) ) {
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 3 );
    // Mark the button press.
    new_button_press = BTN_UP;
    // Process the button press.
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
  }
}

// EXTI channel 4: Nav switch 'down'.
void EXTI4_IRQ_handler( void ) {
  if ( EXTI->PR1 & ( 1 << 4 ) ) {
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 4 );
    // Mark the button press.
    new_button_press = BTN_DOWN;
    // Process the button press.
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
  }
}

// EXTI channels 5-9: Nav switch 'right / left / press', and 'mode'.
void EXTI5_9_IRQ_handler( void ) {
  // PB5: Nav switch 'right'.
  if ( EXTI->PR1 & ( 1 << 5 ) ) {
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 5 );
    // Mark the button press.
    new_button_press = BTN_RIGHT;
    // Process the button press.
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
  }
  // PC6: Nav switch 'left'.
  else if ( EXTI->PR1 & ( 1 << 6 ) ) {
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 6 );
    // Mark the button press.
    new_button_press = BTN_LEFT;
    // Process the button press.
    // Set the background color to green.
    bg_r = 0x00;
    bg_g = 0x2F;
    bg_b = 0x08;
  }
  // PC7: Nav switch 'press / center'.
  else if ( EXTI->PR1 & ( 1 << 7 ) ) {
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 7 );
    // Mark the button press.
    new_button_press = BTN_CENTER;
    // Process the button press.
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
    else if ( cur_mode == MODE_SD_CARD ) {
      // TODO
    }
    else if ( cur_mode == MODE_USB ) {
      // TODO
    }
  }
  // PA8: 'Mode'
  else if ( EXTI->PR1 & ( 1 << 8 ) ) {
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 8 );
    // Mark the button press.
    new_button_press = BTN_MODE;
    // Process the button press.
    // Set the color to teal.
    bg_r = 0x00;
    bg_g = 0x0F;
    bg_b = 0x0F;
  }
}

// EXTI channels 10-15: 'back'.
void EXTI10_15_IRQ_handler( void ) {
  if ( EXTI->PR1 & ( 1 << 14 ) ) {
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 14 );
    // Mark the button press.
    new_button_press = BTN_BACK;
    // Process the button press.
    if ( cur_mode == MODE_GPS_RX ||
         cur_mode == MODE_AUDIO ||
         cur_mode == MODE_BACKLIGHT ||
         cur_mode == MODE_BATTERY ||
         cur_mode == MODE_SD_CARD ||
         cur_mode == MODE_USB ) {
      // TODO: This relies on a specific ordering of the
      // preprocessor macros, and that's bad practice.
      cur_selection = cur_mode - 1;
      // Return to the main menu and reset selection cursor.
      cur_mode = MODE_MAIN_MENU;
    }
  }
}
