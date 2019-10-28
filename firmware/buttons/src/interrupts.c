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
extern uint16_t FRAMEBUFFER[ ILI9341_A ];
extern volatile uint16_t color;

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

// EXTI channel 3: Nav switch 'up'.
void EXTI3_IRQ_handler( void ) {
  if ( EXTI->PR1 & ( 1 << 3 ) ) {
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 3 );
    // Set the color to purple.
    color = rgb565( 0x1F, 0x00, 0x1F );
  }
}

// EXTI channel 4: Nav switch 'down'.
void EXTI4_IRQ_handler( void ) {
  if ( EXTI->PR1 & ( 1 << 4 ) ) {
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 4 );
    // Set the color to yellow.
    color = rgb565( 0x1F, 0x0C, 0x00 );
  }
}

// EXTI channels 5-9: Nav switch 'right / left / press', and 'mode'.
void EXTI5_9_IRQ_handler( void ) {
  if ( EXTI->PR1 & ( 1 << 5 ) ) {
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 5 );
    // Set the color to red.
    color = rgb565( 0x1F, 0x00, 0x00 );
  }
  else if ( EXTI->PR1 & ( 1 << 6 ) ) {
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 6 );
    // Set the color to green.
    color = rgb565( 0x00, 0x3F, 0x00 );
  }
  else if ( EXTI->PR1 & ( 1 << 7 ) ) {
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 7 );
    // Set the color to blue.
    color = rgb565( 0x00, 0x00, 0x1F );
  }
  else if ( EXTI->PR1 & ( 1 << 8 ) ) {
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 8 );
    // Set the color to teal.
    color = rgb565( 0x00, 0x0F, 0x0F );
  }
}

// EXTI channels 10-15: 'back'.
void EXTI10_15_IRQ_handler( void ) {
  if ( EXTI->PR1 & ( 1 << 14 ) ) {
    // Clear the status flag.
    EXTI->PR1 |=  ( 1 << 14 );
    // Set the color to white.
    color = rgb565( 0x1F, 0x3F, 0x1F );
  }
}
