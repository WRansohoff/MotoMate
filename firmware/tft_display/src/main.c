// Standard library includes.
#include <stdint.h>
#include <stdlib.h>
// Vendor-provided device header file.
#include "stm32l4xx.h"
// HAL includes.
#include "hal/dma.h"
#include "hal/gpio.h"
#include "hal/rcc.h"
#include "hal/spi.h"
#include "hal/tim.h"
// BSP includes.
#include "ili9341.h"
#include "ufb.h"

// 320x240-pixel 16-bit (RGB-565) framebuffer. Note: it's 150KB of RAM
uint16_t FRAMEBUFFER[ ILI9341_A ];
uFB framebuffer = {
  w: ILI9341_W,
  h: ILI9341_H,
  buf: ( uint16_t* )&FRAMEBUFFER
};

// Global variable to hold the core clock speed in Hertz.
uint32_t SystemCoreClock = 4000000;

/**
 * Main program.
 */
int main(void) {
  // Initialize the core clock speed to 80MHz.
  clock_init();

  // Enable peripherals: GPIOA, GPIOB, GPIOC, DMA1, SPI1, TIM3.
  RCC->AHB1ENR  |= ( RCC_AHB1ENR_DMA1EN );
  RCC->AHB2ENR  |= ( RCC_AHB2ENR_GPIOAEN |
                     RCC_AHB2ENR_GPIOBEN |
                     RCC_AHB2ENR_GPIOCEN );
  RCC->APB1ENR1 |= ( RCC_APB1ENR1_TIM3EN );
  RCC->APB2ENR  |= ( RCC_APB2ENR_SPI1EN );

  // Setup the NVIC hardware interrupts.
  // Use 4 bits for 'priority' and 0 bits for 'subpriority'.
  NVIC_SetPriorityGrouping( 0x00 );
  // DMA interrupts should be high-priority. (0 is highest)
  uint32_t dma_pri_encoding = NVIC_EncodePriority( 0x00, 0x01, 0x00 );
  NVIC_SetPriority( DMA1_Channel3_IRQn, dma_pri_encoding );
  NVIC_EnableIRQ( DMA1_Channel3_IRQn );

  // Setup GPIO pin(s).
  // PB1: Alt. Func. #2 (TIM3_CH4), low-speed.
  gpio_af_setup( GPIOB, 1, 2, 0 );
  // PB0: Push-pull output (CS pin)
  gpio_setup( GPIOB, 0, GPIO_OUT_PP );
  gpio_hi( GPIOB, 0 );
  // PC5: Push-pull output (TFT Reset pin)
  gpio_setup( GPIOC, 5, GPIO_OUT_PP );
  gpio_hi( GPIOC, 5 );
  // PC4: Push-pull output (Data/Command pin)
  gpio_setup( GPIOC, 4, GPIO_OUT_PP );
  // PA5, PA6, PA7: Alt. Func. #5, mid-speed (SPI1 SCK/SDO/SDI)
  gpio_af_setup( GPIOA, 5, 5, 1 );
  gpio_af_setup( GPIOA, 6, 5, 1 );
  gpio_af_setup( GPIOA, 7, 5, 1 );
  // PA15: 'Heartbeat' LED. (Note: overrides JTDI debugging pin)
  gpio_setup( GPIOA, 15, GPIO_OUT_PP );
  gpio_lo( GPIOA, 15 );

  // Set Timer 3, Channel 4 to a PWM signal with 30% duty cycle.
  // Prescaler
  TIM3->PSC   = 0x0000;
  // Duty cycle = (CCR / ARR).
  TIM3->CCR4  = 3000;
  TIM3->ARR   = 10000;
  // Enable CCR output, set 'PWM mode 1'.
  TIM3->CCER  |=  ( TIM_CCER_CC4E );
  TIM3->CCMR2 &= ~( TIM_CCMR2_OC4M );
  TIM3->CCMR2 |=  ( 0x6 << TIM_CCMR2_OC4M_Pos );
  // Set 'update generation' bit, then start the timer.
  TIM3->EGR   |=  ( TIM_EGR_UG );
  TIM3->CR1   |=  ( TIM_CR1_CEN );

  // DMA configuration (DMA1, channel 3).
  // CCR register:
  // - Memory-to-peripheral
  // - Circular mode disabled.
  // - Increment memory ptr, don't increment periph ptr.
  // - 16-bit data size for both source and destination.
  // - High priority (2/3).
  // - Enable 'transfer complete' interrupt.
  DMA1_Channel3->CCR &= ~( DMA_CCR_MEM2MEM |
                           DMA_CCR_PL |
                           DMA_CCR_CIRC |
                           DMA_CCR_MSIZE |
                           DMA_CCR_PSIZE |
                           DMA_CCR_PINC |
                           DMA_CCR_EN );
  DMA1_Channel3->CCR |=  ( ( 0x2 << DMA_CCR_PL_Pos ) |
                           ( 0x1 << DMA_CCR_MSIZE_Pos ) |
                           ( 0x1 << DMA_CCR_PSIZE_Pos ) |
                           DMA_CCR_MINC |
                           DMA_CCR_TCIE |
                           DMA_CCR_DIR );
  // Set DMA channel selection to pick which of the preset
  // peripheral options we want to use.
  DMA1_CSELR->CSELR &= ~( DMA_CSELR_C3S );
  DMA1_CSELR->CSELR |=  ( 0x1 << DMA_CSELR_C3S_Pos );
  // Set DMA source and destination addresses.
  // Source: Address of the sine wave buffer in memory.
  DMA1_Channel3->CMAR  = ( uint32_t )&FRAMEBUFFER;
  // Dest.: SPI1 data register.
  DMA1_Channel3->CPAR  = ( uint32_t )&( SPI1->DR );
  // Set DMA data transfer length (# of bytes in the framebuffer).
  // TODO: We can't transfer the whole FB, since it is >64K words.
  // So, I'll need to set up a 'transfer complete' interrupt
  // to change the source address and restart the transfer.
  DMA1_Channel3->CNDTR = ( uint16_t )( ILI9341_A / 2 );

  // Setup SPI1 for communicating with the TFT.
  // - Clock phase/polarity: 1/1
  // - Assert internal CS signal (software CS pin control)
  // - MSB-first
  // - 8-bit frames
  // - Baud rate prescaler of 2 (or 128 for debugging)
  // - Enable TX DMA requests.
  SPI1->CR1 &= ~( SPI_CR1_LSBFIRST |
                  SPI_CR1_BR );
  SPI1->CR1 |=  ( SPI_CR1_SSM |
                  SPI_CR1_SSI |
                  0x0 << SPI_CR1_BR_Pos |
                  //0x7 << SPI_CR1_BR_Pos |
                  SPI_CR1_MSTR |
                  SPI_CR1_CPOL |
                  SPI_CR1_CPHA );
  SPI1->CR2 &= ~( SPI_CR2_DS );
  SPI1->CR2 |=  ( 0x7 << SPI_CR2_DS_Pos |
                  SPI_CR2_TXDMAEN );
  // Enable the SPI peripheral.
  SPI1->CR1 |=  ( SPI_CR1_SPE );

  // Send initialization commands.
  // Pull CS pin low.
  gpio_lo( GPIOB, 0 );
  // Software reset.
  dat_cmd( SPI1, ILI9341_CMD );
  spi_w8( SPI1, 0x01 );
  delay_cycles( 200000 );
  // Display off.
  spi_w8( SPI1, 0x28 );
  // Color mode: 16bpp.
  spi_w8( SPI1, 0x3A );
  dat_cmd( SPI1, ILI9341_DAT );
  spi_w8( SPI1, 0x55 );
  // Memory access control: flip display across the 240px axis.
  dat_cmd( SPI1, ILI9341_CMD );
  spi_w8( SPI1, 0x36 );
  dat_cmd( SPI1, ILI9341_DAT );
  spi_w8( SPI1, 0x40 );
  // Exit sleep mode.
  dat_cmd( SPI1, ILI9341_CMD );
  spi_w8( SPI1, 0x11 );
  delay_cycles( 200000 );
  // Display on.
  spi_w8( SPI1, 0x29 );
  delay_cycles( 200000 );
  // Set drawing window.
  // Column set: [0:239]
  spi_w8( SPI1, 0x2A );
  dat_cmd( SPI1, ILI9341_DAT );
  spi_w16( SPI1, 0x0000 );
  spi_w16( SPI1, ( ( 239 & 0xFF ) << 8 ) | ( 239 >> 8 ) );
  dat_cmd( SPI1, ILI9341_CMD );
  // Row set: [0:319]
  spi_w8( SPI1, 0x2B );
  dat_cmd( SPI1, ILI9341_DAT );
  spi_w16( SPI1, 0x0000 );
  spi_w16( SPI1, ( ( 319 & 0xFF ) << 8 ) | ( 319 >> 8 ) );
  dat_cmd( SPI1, ILI9341_CMD );
  // Set 'write to RAM' mode.
  spi_w8( SPI1, 0x2C );
  // From now on, we'll only be sending pixel data.
  dat_cmd( SPI1, ILI9341_DAT );

  // Enable DMA1 Channel 3.
  DMA1_Channel3->CCR |= ( DMA_CCR_EN );

  // Done; now just alternate between solid colors to get
  // a feel for the refresh speed.
  uint16_t color = rgb565( 0x1F, 0x00, 0x1F );
  uint16_t r_col = rgb565( 0x1F, 0x08, 0x00 );
  while (1) {
    // Draw the new color to the framebuffer.
    ufb_fill_rect( &framebuffer, color, 0, 0, 240, 320 );
    ufb_draw_rect( &framebuffer, ( color ^ 0xFFFF ), 0, 0, 240, 320, 4 );
    ufb_draw_circle( &framebuffer, ( color ^ 0xFFFF ), 120, 160, 30, 8 );
    ufb_fill_circle( &framebuffer, ( color ^ 0xFFFF ), 40, 40, 15 );
    ufb_fill_circle( &framebuffer, ( color ^ 0xFFFF ), 200, 260, 50 );
    ufb_draw_rect( &framebuffer, r_col, -5, 285, 50, 50, 6 );
    ufb_fill_circle( &framebuffer, ( color ^ 0xFFFF ), 20, 310, 25 );
    ufb_draw_str( &framebuffer, ( color ^ 0xFFFF ), 20, 20, "Hello\0", 2, UFB_ORIENT_H );
    ufb_draw_str( &framebuffer, ( color ^ 0xFFFF ), 15, 10, "Hello\0", 1, UFB_ORIENT_V );
    ufb_draw_str( &framebuffer, r_col, 180, 30, "Test\0", 7, UFB_ORIENT_H );
    //ufb_fill_rect( &framebuffer, ( color ^ 0xFFFF ), 200, 20, 6, 8 );
    // Invert the color.
    color = color ^ 0xFFFF;
    // Delay briefly.
    delay_cycles( 5000000 );
    // Toggle the 'heartbeat' LED.
    //gpio_toggle( GPIOA, 15 );
  }
}

/* Interrupt Handlers */
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
