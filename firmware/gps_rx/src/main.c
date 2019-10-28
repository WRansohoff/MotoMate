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

// 320x240-pixel 16-bit (RGB-565) framebuffer. Note: it's 150KB of RAM.
uint16_t FRAMEBUFFER[ ILI9341_A ];
uFB framebuffer = {
  w: ILI9341_W,
  h: ILI9341_H,
  buf: ( uint16_t* )&FRAMEBUFFER
};

// Ringbuffer for holding data sent by the GPS module.
#define GPS_RINGBUF_LEN ( 1024 )
char gps_rb_buf[ GPS_RINGBUF_LEN + 1 ];
ringbuf gps_rb = {
  len: GPS_RINGBUF_LEN,
  buf: gps_rb_buf,
  pos: 0,
  ext: 0
};

// Global variable to hold the core clock speed in Hertz.
uint32_t SystemCoreClock = 4000000;

// Send a string over UART (blocking)
void uart_tx_str( USART_TypeDef* UARTx, const unsigned char* str, size_t len ) {
  while( len-- ) {
    while( !( UARTx->ISR & USART_ISR_TXE ) ) {};
    UARTx->TDR = *str++;
  }
}

/**
 * Main program.
 */
int main(void) {
  // Initialize the core clock speed to 80MHz.
  clock_init();

  // Enable peripherals: GPIOA, GPIOB, GPIOC, DMA1, SPI1, TIM3, UART4.
  RCC->AHB1ENR  |=  ( RCC_AHB1ENR_DMA1EN );
  RCC->AHB2ENR  |=  ( RCC_AHB2ENR_GPIOAEN |
                      RCC_AHB2ENR_GPIOBEN |
                      RCC_AHB2ENR_GPIOCEN );
  RCC->APB1ENR1 |=  ( RCC_APB1ENR1_TIM3EN |
                      RCC_APB1ENR1_UART4EN );
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
  // UART receive interrupts should be highest-priority.
  uint32_t uart_pri_encoding = NVIC_EncodePriority( 0x00, 0x00, 0x00 );
  NVIC_SetPriority( UART4_IRQn, uart_pri_encoding );
  NVIC_EnableIRQ( UART4_IRQn );

  // Setup GPIO pins.
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
  // PA0, PA1: Alt. Func. #8, low-speed (UART4 TX, RX)
  gpio_af_setup( GPIOA, 0, 8, 0 );
  gpio_af_setup( GPIOA, 1, 8, 0 );
  // PA5, PA6, PA7: Alt. Func. #5, mid-speed (SPI1 SCK/SDO/SDI)
  gpio_af_setup( GPIOA, 5, 5, 1 );
  gpio_af_setup( GPIOA, 6, 5, 1 );
  gpio_af_setup( GPIOA, 7, 5, 1 );
  // PA15: 'Heartbeat' LED. (Note: overrides JTDI debugging pin)
  gpio_setup( GPIOA, 15, GPIO_OUT_PP );
  gpio_lo( GPIOA, 15 );

  // Ensure there is always a null terminator at the end of the
  // UART receive ringbuffer.
  gps_rb.buf[ GPS_RINGBUF_LEN ] = '\0';
  // Initialize UART4 for 9600-baud communication.
  // Set baud rate.
  UART4->BRR   =  ( SystemCoreClock / 9600 );
  // Set and enable RX timeout.
  // I think that this tells it to trigger a timeout after 10
  // 1-bit-long cycles have elapsed. So in this case, every 960Hz?
  UART4->RTOR &= ~( USART_RTOR_RTO );
  UART4->RTOR |=  ( 10 << USART_RTOR_RTO_Pos );
  UART4->CR2  |=  ( USART_CR2_RTOEN );
  // Enable the TX and RX lines, the RX interrupt,
  // the RX timeout interrupt, and the peripheral.
  UART4->CR1  |=  ( USART_CR1_UE |
                    USART_CR1_RE |
                    USART_CR1_RXNEIE |
                    USART_CR1_RTOIE |
                    USART_CR1_TE );

  // Set Timer 3, Channel 4 to a 1MHz PWM signal with 5% duty cycle.
  timer_pwm_out( TIM3, 4, 0.05, 1000000 );

  // DMA configuration (DMA1, channel 3).
  dma_config_tx_single( DMA1_BASE, 3,
                        ( uint32_t )&FRAMEBUFFER,
                        ( uint32_t )&( SPI1->DR ),
                        ( uint16_t )( ILI9341_A / 2 ),
                        1, DMA_PRIORITY_HI, DMA_SIZE_16b, 1 );

  // Setup SPI1 for communicating with the TFT.
  spi_host_init( SPI1, 0, 1 );

  // Send initialization commands to the display.
  ili9341_init( SPI1 );

  // Enable DMA1 Channel 3.
  DMA1_Channel3->CCR |= ( DMA_CCR_EN );

  // Done; now periodically update the framebuffer
  // with the contents of the GPS UART receive buffer.
  // (The buffer is populated in src/interrupts.c)
  uint16_t color = rgb565( 0x1F, 0x00, 0x1F );
  while (1) {
    // Clear the display.
    ufb_fill_rect( &framebuffer, color, 0, 0, 240, 320 );
    // Print the current ringbuffer.
    ufb_draw_lines( &framebuffer, ( color ^ 0xFFFF ), 8, 0, ( char* )gps_rb.buf, 1, UFB_ORIENT_H );
    // Delay briefly.
    delay_cycles( 5000000 );
    // Toggle the 'heartbeat' LED.
    //gpio_toggle( GPIOA, 15 );
  }
}
