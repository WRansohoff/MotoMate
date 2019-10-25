/*
 * Minimal SPI interface methods.
 */
#ifndef __VVC_SPI
#define __VVC_SPI

// Device header file.
#include "stm32l4xx.h"

// Write a byte to the SPI peripheral. (TODO: Macro?)
void spi_w8( SPI_TypeDef *SPIx, uint8_t dat );

// Write two bytes to the SPI peripheral. Note that they
// send in the order of 0x2211. (1 = first, 2 = second)
// TODO: Macro?
void spi_w16( SPI_TypeDef *SPIx, uint16_t dat );

// Simple SPI host initialization with some commonly-used settings.
// * CPHA/CPOL = 1/1
// * CS pin controlled by the application software.
// * MSB-first
// Options:
// * brr:   Baud rate prescaler. (2x-128x) Must be in the range [0:7].
// * dmaen: 0 to disable TX DMA requests, 1 to enable.
void spi_host_init( SPI_TypeDef *SPIx, int brr, int dmaen );

#endif
