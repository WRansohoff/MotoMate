/*
 * (Interrupt handlers go here)
 */

// Standard library includes.
#include <stdint.h>

// Vendor-provided device header file.
#include "stm32l4xx.h"

// HAL includes.
#include "hal/rcc.h"
#include "hal/tim.h"

// BSP includes.
#include "ili9341.h"
#include "ringbuf.h"
#include "ufb.h"

// Project includes.
#include "app.h"
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

// EXTI channel 3: Nav switch 'down'.
void EXTI3_IRQ_handler( void ) {
  if ( EXTI->PR1 & ( 1 << 3 ) ) {
    // Mark the button press.
    register_button_press( BTN_DOWN );
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 3 );
  }
}

// EXTI channel 4: Nav switch 'up'.
void EXTI4_IRQ_handler( void ) {
  if ( EXTI->PR1 & ( 1 << 4 ) ) {
    // Mark the button press.
    register_button_press( BTN_UP );
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 4 );
  }
}

// EXTI channels 5-9: Nav switch 'right / left / press', and 'mode'.
void EXTI5_9_IRQ_handler( void ) {
  // PB5: Nav switch 'right'.
  if ( EXTI->PR1 & ( 1 << 5 ) ) {
    // Mark the button press.
    register_button_press( BTN_RIGHT );
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 5 );
  }
  // PC6: Nav switch 'left'.
  else if ( EXTI->PR1 & ( 1 << 6 ) ) {
    // Mark the button press.
    register_button_press( BTN_LEFT );
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 6 );
  }
  // PC7: Nav switch 'press / center'.
  else if ( EXTI->PR1 & ( 1 << 7 ) ) {
    // Mark the button press.
    register_button_press( BTN_CENTER );
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 7 );
  }
  // PA8: 'Mode'
  else if ( EXTI->PR1 & ( 1 << 8 ) ) {
    // Mark the button press.
    register_button_press( BTN_MODE );
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 8 );
  }
}

// EXTI channels 10-15: 'back'.
void EXTI10_15_IRQ_handler( void ) {
  if ( EXTI->PR1 & ( 1 << 14 ) ) {
    // Mark the button press.
    register_button_press( BTN_BACK );
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 14 );
  }
}
