/*
 * Minimal RCC (Reset & Clock Control) interface methods.
 */
#include "hal/rcc.h"

// Simple imprecise delay method.
void __attribute__( ( optimize( "O0" ) ) )
delay_cycles( uint32_t cyc ) {
  for ( uint32_t d_i = 0; d_i < cyc; ++d_i ) { asm( "NOP" ); }
}

// Placeholder method to set the core clock speed to 80MHz.
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
  // For 80MHz, R = 2, M = 1, N = 10.
  RCC->PLLCFGR &= ~( RCC_PLLCFGR_PLLN |
                     RCC_PLLCFGR_PLLM |
                     RCC_PLLCFGR_PLLR |
                     RCC_PLLCFGR_PLLREN |
                     RCC_PLLCFGR_PLLPEN |
                     RCC_PLLCFGR_PLLQEN |
                     RCC_PLLCFGR_PLLSRC );
  RCC->PLLCFGR |=  ( RCC_PLLCFGR_PLLREN |
                     10 << RCC_PLLCFGR_PLLN_Pos |
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
