/*
 * Minimal library for controlling ILI9341 displays
 * using a minimal framebuffer library and STM32 HAL.
 */
#include "ili9341.h"

// Method to set the 'data / command' pin.
// TODO: Don't hardcode the pin.
void dat_cmd( SPI_TypeDef *SPIx, uint8_t dc ) {
  // Wait for the BSY 'busy' bit to be cleared.
  while ( SPIx->SR & SPI_SR_BSY ) {};
  // Set the D/C pin appropriately.
  if ( dc ) { GPIOC->ODR |=  ( 1 << 4 ); }
  else      { GPIOC->ODR &= ~( 1 << 4 ); }
}

// Simple minimal initialization commands for testing.
// TODO: This could use some sort of struct of dat/cmd values.
void ili9341_init( SPI_TypeDef *SPIx ) {
  // Pull CS pin low.
  gpio_lo( GPIOB, 0 );
  // Software reset.
  dat_cmd( SPIx, ILI9341_CMD );
  spi_w8( SPIx, 0x01 );
  delay_cycles( 200000 );
  // Display off.
  spi_w8( SPIx, 0x28 );
  // Color mode: 16bpp.
  spi_w8( SPIx, 0x3A );
  dat_cmd( SPIx, ILI9341_DAT );
  spi_w8( SPIx, 0x55 );
  // Memory access control: flip display across the 240px axis.
  dat_cmd( SPIx, ILI9341_CMD );
  spi_w8( SPIx, 0x36 );
  dat_cmd( SPIx, ILI9341_DAT );
  spi_w8( SPIx, 0x40 );
  // Exit sleep mode.
  dat_cmd( SPIx, ILI9341_CMD );
  spi_w8( SPIx, 0x11 );
  delay_cycles( 200000 );
  // Display on.
  spi_w8( SPIx, 0x29 );
  delay_cycles( 200000 );
  // Set drawing window.
  // Column set: [0:239]
  spi_w8( SPIx, 0x2A );
  dat_cmd( SPIx, ILI9341_DAT );
  spi_w16( SPIx, 0x0000 );
  spi_w16( SPIx, ( ( ILI9341_W & 0xFF ) << 8 ) | ( ILI9341_W >> 8 ) );
  dat_cmd( SPIx, ILI9341_CMD );
  // Row set: [0:319]
  spi_w8( SPIx, 0x2B );
  dat_cmd( SPIx, ILI9341_DAT );
  spi_w16( SPIx, 0x0000 );
  spi_w16( SPIx, ( ( ILI9341_H & 0xFF ) << 8 ) | ( ILI9341_H >> 8 ) );
  dat_cmd( SPIx, ILI9341_CMD );
  // Set 'write to RAM' mode.
  spi_w8( SPIx, 0x2C );
  // From now on, we'll only be sending pixel data.
  dat_cmd( SPIx, ILI9341_DAT );
}
