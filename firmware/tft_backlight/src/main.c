// Standard library includes.
#include <stdint.h>
#include <stdlib.h>
// Vendor-provided device header file.
#include "stm32l4xx.h"

// Global variable to hold the core clock speed in Hertz.
uint32_t SystemCoreClock = 4000000;

// Simple imprecise delay method.
void __attribute__( ( optimize( "O0" ) ) )
delay_cycles( uint32_t cyc ) {
  for ( uint32_t d_i = 0; d_i < cyc; ++d_i ) { asm( "NOP" ); }
}

// Method to set the core clock speed to 80MHz.
void clock_init( void ) {
  // Flash settings:
  // * Set 3 wait states (use 4 instead if operating at 1.8V)
  // * (Data cache and instruction cache are enabled by default).
  // * Enable prefetching.
  FLASH->ACR &= ~( FLASH_ACR_LATENCY );
  FLASH->ACR |=  ( ( 0x3 << FLASH_ACR_LATENCY_Pos ) |
                   FLASH_ACR_PRFTEN );
  // Enable HSI16 oscillator.
  RCC->CR    |=  ( RCC_CR_HSION );
  // PLL configuration: frequency = ( 16MHz * ( N / M ) ) / R
  // For 80MHz, R = 2, M = 1, N = 16.
  RCC->PLLCFGR &= ~( RCC_PLLCFGR_PLLN |
                     RCC_PLLCFGR_PLLM |
                     RCC_PLLCFGR_PLLR |
                     RCC_PLLCFGR_PLLREN |
                     RCC_PLLCFGR_PLLPEN |
                     RCC_PLLCFGR_PLLQEN |
                     RCC_PLLCFGR_PLLSRC );
  RCC->PLLCFGR |=  ( RCC_PLLCFGR_PLLREN |
                     16 << RCC_PLLCFGR_PLLN_Pos |
                     2  << RCC_PLLCFGR_PLLSRC_Pos );
  // Wait for the HSI oscillator to be ready.
  while ( !( RCC->CR & RCC_CR_HSIRDY ) ) {};
  // Enable the PLL and select it as the system clock source.
  RCC->CR   |=  ( RCC_CR_PLLON );
  while ( !( RCC->CR & RCC_CR_PLLRDY ) ) {};
  RCC->CFGR &= ~( RCC_CFGR_SW );
  RCC->CFGR |=  ( 0x3 << RCC_CFGR_SW_Pos );
  while ( ( RCC->CFGR & RCC_CFGR_SWS ) != ( 0x3 << RCC_CFGR_SWS_Pos ) ) {};
  // System clock is now 80MHz.
  SystemCoreClock = 80000000;
  // Disable the MSI oscillator, since it is no longer being used.
  // Note: MSI range 8 uses ~90uA less current, but is less accurate.
  RCC->CR   &= ~( RCC_CR_MSION );
}

/**
 * Main program.
 */
int main(void) {
  // Initialize the core clock to 80MHz.
  clock_init();

  // Enable peripheral(s): GPIOB, TIM3.
  RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOBEN;
  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN;

  // Setup GPIO pin(s).
  // PB1: Alt. Func. #2 (TIM3_CH4).
  GPIOB->MODER   &= ~( 0x3 << ( 1 * 2 ) );
  GPIOB->MODER   |=  ( 0x2 << ( 1 * 2 ) );
  GPIOB->OSPEEDR &= ~( 0x3 << ( 1 * 2 ) );
  GPIOB->OTYPER  &= ~( 0x1 << 1 );
  GPIOB->PUPDR   &= ~( 0x1 << 1 );
  GPIOB->AFR[0]  &= ~( 0xF << ( 1 * 4 ) );
  GPIOB->AFR[0]  |=  ( 0x2 << ( 1 * 4 ) );

  // Set Timer 3, Channel 4 to a PWM signal with 50% duty cycle.
  // Prescaler
  TIM3->PSC   = 0x0000;
  // Duty cycle = (CCR / ARR).
  uint16_t duty_cycle = 0x0100;
  TIM3->CCR4  = duty_cycle;
  TIM3->ARR   = 0x0200;
  // Enable CCR output, set 'PWM mode 1'.
  TIM3->CCER  |=  ( TIM_CCER_CC4E );
  TIM3->CCMR2 &= ~( TIM_CCMR2_OC4M );
  TIM3->CCMR2 |=  ( 0x6 << TIM_CCMR2_OC4M_Pos );
  // Set 'update generation' bit, then start the timer.
  TIM3->EGR   |=  ( TIM_EGR_UG );
  TIM3->CR1   |=  ( TIM_CR1_CEN );

  // Main loop.
  int dir = 1;
  uint16_t step = 0x0010;
  while (1) {
    // Pulse the display's brightness.
    delay_cycles( 400000 );
    if ( ( duty_cycle > ( 0x0200 - step ) ) ||
         ( duty_cycle < ( 0x0000 + step ) ) ) {
      dir = -dir;
    }
    duty_cycle += ( dir * step );
    TIM3->CCR4 = duty_cycle;
  }
}
