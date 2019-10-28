// Standard library includes.
#include <stdint.h>
#include <stdlib.h>
// Vendor-provided device header file.
#include "stm32l4xx.h"
// HAL includes.
#include "hal/adc.h"
#include "hal/dma.h"
#include "hal/gpio.h"
#include "hal/rcc.h"
#include "hal/spi.h"
#include "hal/tim.h"
// BSP includes.
#include "ili9341.h"
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

// Global variable to hold the core clock speed in Hertz.
uint32_t SystemCoreClock = 4000000;

/**
 * Main program.
 */
int main(void) {
  // Initialize the core clock speed to 80MHz.
  clock_init();

  // Enable peripherals: GPIOA, GPIOB, GPIOC, DMA1, SPI1, TIM3, ADC.
  RCC->AHB1ENR  |=  ( RCC_AHB1ENR_DMA1EN );
  RCC->AHB2ENR  |=  ( RCC_AHB2ENR_GPIOAEN |
                      RCC_AHB2ENR_GPIOBEN |
                      RCC_AHB2ENR_GPIOCEN |
                      RCC_AHB2ENR_ADCEN );
  RCC->APB1ENR1 |=  ( RCC_APB1ENR1_TIM3EN );
  RCC->APB2ENR  |=  ( RCC_APB2ENR_SPI1EN );

  // Enable FPU. (Set Coprocessors 1 & 2 to 'full access')
  // Trying to use floats before this is called will cause a crash.
  SCB->CPACR    |=  ( 0xF << 20 );

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
  // PC2: Analog mode (battery check ADC input)
  gpio_setup( GPIOC, 2, GPIO_ANALOG );
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

  // Set Timer 3, Channel 4 to a 1MHz PWM signal with 30% duty cycle.
  timer_pwm_out( TIM3, 4, 0.3, 1000000 );

  // Select the system clock as the ADC clock source.
  adc_clock_source( ADC_CLOCK_SYSCLK );
  // Turn on the ADC and calibrate it.
  adc_on( ADC1, 1 );
  // Setup ADC for a single conversion on channel 3.
  adc_conversion battery_adc = {
    channel: 3,
    sample_time: ADC_SAMP_640_CYC
  };
  adc_sequence_config( ADC1, &battery_adc, 1 );
  // Enable the ADC.
  ADC1->CR   |=   ( ADC_CR_ADEN );

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

  // Wait for the ADC to be ready.
  while ( !( ADC1->ISR & ADC_ISR_ADRDY ) ) {};

  // Done; now just alternate between solid colors to get
  // a feel for the refresh speed.
  uint16_t color = rgb565( 0x1F, 0x00, 0x1F );
  uint16_t adc_val = 0x0000;
  while (1) {
    // Clear the display.
    ufb_fill_rect( &framebuffer, color, 0, 0, 240, 320 );
    // Read the ADC.
    adc_val = adc_single_conversion( ADC1 );
    // Draw the ADC value to the display. It'll be between [0:4096].
    ufb_draw_int( &framebuffer, ( color ^ 0xFFFF ), 120, 160, adc_val, 3, UFB_ORIENT_H );
    // TODO: Make a 'draw float' method and show estimated battery voltage.
    // Delay briefly.
    delay_cycles( 5000000 );
    // Toggle the 'heartbeat' LED.
    //gpio_toggle( GPIOA, 15 );
  }
}
