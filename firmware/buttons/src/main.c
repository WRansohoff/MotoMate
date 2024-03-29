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
#include "ringbuf.h"
#include "ufb.h"

// Dummy 'current audio samples' value for HAL library.
volatile int cur_samples = 32;

// 320x240-pixel 16-bit (RGB-565) framebuffer. Note: it's 150KB of RAM.
uint16_t FRAMEBUFFER[ ILI9341_A ];
uFB framebuffer = {
  w: ILI9341_W,
  h: ILI9341_H,
  buf: ( uint16_t* )&FRAMEBUFFER
};

// Solid color to set the display to; button presses will change this.
volatile uint16_t color = rgb565( 0x1F, 0x00, 0x1F );

// Global variable to hold the core clock speed in Hertz.
uint32_t SystemCoreClock = 4000000;

/**
 * Main program.
 */
int main(void) {
  // Initialize the core clock speed to 80MHz.
  clock_init();

  // Enable peripherals: GPIOA, GPIOB, GPIOC,
  // DMA1, SPI1, TIM3, SYSCFG.
  RCC->AHB1ENR  |=  ( RCC_AHB1ENR_DMA1EN );
  RCC->AHB2ENR  |=  ( RCC_AHB2ENR_GPIOAEN |
                      RCC_AHB2ENR_GPIOBEN |
                      RCC_AHB2ENR_GPIOCEN );
  RCC->APB1ENR1 |=  ( RCC_APB1ENR1_TIM3EN );
  RCC->APB2ENR  |=  ( RCC_APB2ENR_SPI1EN |
                      RCC_APB2ENR_SYSCFGEN );

  // Enable FPU. (Set Coprocessors 1 & 2 to 'full access')
  // Trying to use floats before this is called will cause a crash.
  SCB->CPACR    |=  ( 0xF << 20 );

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
  // Setup button pins as inputs with pull-up resistors enabled.
  // PB3, PB4, PB5, C6, C7: Navigation switch button inputs.
  gpio_setup( GPIOB, 3, GPIO_IN_PU );
  gpio_setup( GPIOB, 4, GPIO_IN_PU );
  gpio_setup( GPIOB, 5, GPIO_IN_PU );
  gpio_setup( GPIOC, 6, GPIO_IN_PU );
  gpio_setup( GPIOC, 7, GPIO_IN_PU );
  // PA8: 'Mode' button input.
  gpio_setup( GPIOA, 8, GPIO_IN_PU );
  // PB14: 'Back' button input.
  gpio_setup( GPIOB, 14, GPIO_IN_PU );

  // Enable EXTI interrupts for button pins.
  SYSCFG->EXTICR[ 0 ] &= ~( SYSCFG_EXTICR1_EXTI3_Msk );
  SYSCFG->EXTICR[ 0 ] |=  ( SYSCFG_EXTICR1_EXTI3_PB );
  SYSCFG->EXTICR[ 1 ] &= ~( SYSCFG_EXTICR2_EXTI4_Msk |
                            SYSCFG_EXTICR2_EXTI5_Msk |
                            SYSCFG_EXTICR2_EXTI6_Msk |
                            SYSCFG_EXTICR2_EXTI7_Msk );
  SYSCFG->EXTICR[ 1 ] |=  ( SYSCFG_EXTICR2_EXTI4_PB |
                            SYSCFG_EXTICR2_EXTI5_PB |
                            SYSCFG_EXTICR2_EXTI6_PC |
                            SYSCFG_EXTICR2_EXTI7_PC );
  SYSCFG->EXTICR[ 2 ] &= ~( SYSCFG_EXTICR3_EXTI8_Msk );
  SYSCFG->EXTICR[ 2 ] |=  ( SYSCFG_EXTICR3_EXTI8_PA );
  SYSCFG->EXTICR[ 3 ] &= ~( SYSCFG_EXTICR4_EXTI14_Msk );
  SYSCFG->EXTICR[ 3 ] |=  ( SYSCFG_EXTICR4_EXTI14_PB );
  // Enable interrupts for the given pins.
  EXTI->IMR1          |=  ( EXTI_IMR1_IM3 |
                            EXTI_IMR1_IM4 |
                            EXTI_IMR1_IM5 |
                            EXTI_IMR1_IM6 |
                            EXTI_IMR1_IM7 |
                            EXTI_IMR1_IM8 |
                            EXTI_IMR1_IM14 );
  // Disable 'rising edge' interrupts.
  EXTI->RTSR1         &= ~( EXTI_RTSR1_RT3 |
                            EXTI_RTSR1_RT4 |
                            EXTI_RTSR1_RT5 |
                            EXTI_RTSR1_RT6 |
                            EXTI_RTSR1_RT7 |
                            EXTI_RTSR1_RT8 |
                            EXTI_RTSR1_RT14 );
  // Enable 'falling edge' interrupts.
  EXTI->FTSR1         |=  ( EXTI_FTSR1_FT3 |
                            EXTI_FTSR1_FT4 |
                            EXTI_FTSR1_FT5 |
                            EXTI_FTSR1_FT6 |
                            EXTI_FTSR1_FT7 |
                            EXTI_FTSR1_FT8 |
                            EXTI_FTSR1_FT14 );

  // Setup the NVIC hardware interrupts.
  // Use 4 bits for 'priority' and 0 bits for 'subpriority'.
  NVIC_SetPriorityGrouping( 0x00 );
  // DMA interrupts should be high-priority. (0 is highest)
  uint32_t dma_pri_encoding = NVIC_EncodePriority( 0x00, 0x01, 0x00 );
  NVIC_SetPriority( DMA1_Channel3_IRQn, dma_pri_encoding );
  NVIC_EnableIRQ( DMA1_Channel3_IRQn );
  // Button inputs should be fairly low priority (15 is lowest)
  uint32_t btn_pri_encoding = NVIC_EncodePriority( 0x00, 0x0C, 0x00 );
  NVIC_SetPriority( EXTI3_IRQn, btn_pri_encoding );
  NVIC_EnableIRQ( EXTI3_IRQn );
  NVIC_SetPriority( EXTI4_IRQn, btn_pri_encoding );
  NVIC_EnableIRQ( EXTI4_IRQn );
  NVIC_SetPriority( EXTI9_5_IRQn, btn_pri_encoding );
  NVIC_EnableIRQ( EXTI9_5_IRQn );
  NVIC_SetPriority( EXTI15_10_IRQn, btn_pri_encoding );
  NVIC_EnableIRQ( EXTI15_10_IRQn );

  // Set Timer 3, Channel 4 to a 1MHz PWM signal with 30% duty cycle.
  timer_pwm_out( TIM3, 4, 0.3, 1000000 );

  // DMA configuration (DMA1, channel 3).
  dma_config_tx( DMA1_BASE, 3,
                 ( uint32_t )&FRAMEBUFFER,
                 ( uint32_t )&( SPI1->DR ),
                 ( uint16_t )( ILI9341_A / 2 ),
                 1, DMA_PRIORITY_HI, DMA_SIZE_16b, 0, 1 );

  // Setup SPI1 for communicating with the TFT.
  spi_host_init( SPI1, 0, 1 );

  // Send initialization commands to the display.
  ili9341_init( SPI1 );

  // Enable DMA1 Channel 3.
  DMA1_Channel3->CCR |= ( DMA_CCR_EN );

  // Done.
  while (1) {
    // Clear the display.
    ufb_fill_rect( &framebuffer, color, 0, 0, 240, 320 );
    // Delay briefly.
    delay_cycles( 50000 );
    // Toggle the 'heartbeat' LED.
    //gpio_toggle( GPIOA, 15 );
  }
}
