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

// Declarations for values that are defined somewhere else.
extern volatile uint16_t FRAMEBUFFER[ ILI9341_A ];
extern ringbuf gps_rb;
extern volatile uint16_t bg_r, bg_g, bg_b;

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
    // Update the source address and refill the 'count' register.
    if ( DMA1_Channel3->CMAR >= ( uint32_t )&( FRAMEBUFFER[ ILI9341_A / 2 ] ) ) {
      DMA1_Channel3->CMAR  = ( uint32_t )&FRAMEBUFFER;
    }
    else {
      DMA1_Channel3->CMAR  = ( uint32_t )&( FRAMEBUFFER[ ILI9341_A / 2 ] );
    }
    DMA1_Channel3->CNDTR = ( uint16_t )( ILI9341_A / 2 );
    // Re-enable the DMA channel to resume the transfer.
    DMA1_Channel3->CCR |=  ( DMA_CCR_EN );
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
    // Acknowledge the interrupt.
    UART4->ICR |=  ( USART_ICR_RTOCF );
  }
}

// EXTI channel 3: Nav switch 'up'.
void EXTI3_IRQ_handler( void ) {
  if ( EXTI->PR1 & ( 1 << 3 ) ) {
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 3 );
    // Set the color to purple.
    bg_r = 0x1F;
    bg_g = 0x00;
    bg_b = 0x1F;
  }
}

// EXTI channel 4: Nav switch 'down'.
void EXTI4_IRQ_handler( void ) {
  if ( EXTI->PR1 & ( 1 << 4 ) ) {
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 4 );
    // Set the color to yellow.
    bg_r = 0x1F;
    bg_g = 0x0C;
    bg_b = 0x00;
  }
}

// EXTI channels 5-9: Nav switch 'right / left / press', and 'mode'.
void EXTI5_9_IRQ_handler( void ) {
  if ( EXTI->PR1 & ( 1 << 5 ) ) {
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 5 );
    // Set the color to red.
    bg_r = 0x1F;
    bg_g = 0x00;
    bg_b = 0x00;
  }
  else if ( EXTI->PR1 & ( 1 << 6 ) ) {
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 6 );
    // Set the color to green.
    bg_r = 0x00;
    bg_g = 0x3F;
    bg_b = 0x00;
  }
  else if ( EXTI->PR1 & ( 1 << 7 ) ) {
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 7 );
    // Set the color to blue.
    bg_r = 0x00;
    bg_g = 0x00;
    bg_b = 0x1F;
  }
  else if ( EXTI->PR1 & ( 1 << 8 ) ) {
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 8 );
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
    // Set the color to white.
    bg_r = 0x1F;
    bg_g = 0x3F;
    bg_b = 0x1F;
  }
}
