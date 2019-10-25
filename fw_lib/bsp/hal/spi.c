/*
 * Minimal SPI interface methods.
 */
#include "hal/spi.h"

// Write a byte to the SPI peripheral. (TODO: Macro?)
void spi_w8( SPI_TypeDef *SPIx, uint8_t dat ) {
  // Wait for TXE 'transmit buffer empty' bit to be set.
  while ( !( SPIx->SR & SPI_SR_TXE ) ) {};
  // Send the byte.
  *( uint8_t* )&( SPIx->DR ) = dat;
}

// Write two bytes to the SPI peripheral. Note that they
// send in the order of 0x2211. (1 = first, 2 = second)
// TODO: Macro?
void spi_w16( SPI_TypeDef *SPIx, uint16_t dat ) {
  // Wait for TXE 'transmit buffer empty' bit to be set.
  while ( !( SPIx->SR & SPI_SR_TXE ) ) {};
  // Send the bytes.
  *( uint16_t* )&( SPIx->DR ) = dat;
}

// Simple SPI host initialization with some commonly-used settings.
// * CPHA/CPOL = 1/1
// * CS pin controlled by the application software.
// * MSB-first
// Options:
// * brr:   Baud rate prescaler. (2x-128x) Must be in the range [0:7].
// * dmaen: 0 to disable TX DMA requests, 1 to enable.
void spi_host_init( SPI_TypeDef *SPIx, int brr, int dmaen ) {
  SPI1->CR1 &= ~( SPI_CR1_LSBFIRST |
                  SPI_CR1_BR );
  SPI1->CR1 |=  ( SPI_CR1_SSM |
                  SPI_CR1_SSI |
                  ( brr & 0x7 ) << SPI_CR1_BR_Pos |
                  SPI_CR1_MSTR |
                  SPI_CR1_CPOL |
                  SPI_CR1_CPHA );
  SPI1->CR2 &= ~( SPI_CR2_DS );
  SPI1->CR2 |=  ( 0x7 << SPI_CR2_DS_Pos |
                  ( !!dmaen ) << SPI_CR2_TXDMAEN_Pos );
  // Enable the SPI peripheral.
  SPI1->CR1 |=  ( SPI_CR1_SPE );
}
