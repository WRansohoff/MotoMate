/*
 * Minimal library for controlling ILI9341 displays
 * using a minimal framebuffer library and STM32 HAL.
 */
#ifndef __VVC_ILI9341
#define __VVC_ILI9341

// C library includes.
#include <stdint.h>

// HAL includes.
#include "hal/rcc.h"
#include "hal/gpio.h"
#include "hal/spi.h"

// Preprocessor definitions for an ILI9341 display's resolution.
#define ILI9341_W ( 240 )
#define ILI9341_H ( 320 )
#define ILI9341_A ( ILI9341_W * ILI9341_H )
// Macro definitions for 'command' (0) and 'data' (1) modes.
#define ILI9341_CMD ( 0 )
#define ILI9341_DAT ( 1 )

// Method to set the 'data / command' pin.
// TODO: Don't hardcode the pin.
void dat_cmd( SPI_TypeDef *SPIx, uint8_t dc );

// Simple minimal initialization commands for testing.
// TODO: This could use some sort of struct of dat/cmd values.
void ili9341_init( SPI_TypeDef *SPIx );

#endif
