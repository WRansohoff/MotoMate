/*
 * Minimal UART interface methods.
 */
#ifndef __VVC_UART
#define __VVC_UART

// Standard library includes (for `size_t` definition).
#include <stdlib.h>

// Device header file.
#include "stm32l4xx.h"

// Variables that are defined elsewhere.
extern uint32_t SystemCoreClock;

// Enable a UART peripheral with a given baud rate
// and receive timeout, with asynchronous receives.
// RXNE/RTOF interrupts should be defined and NVIC interrupt enabled.
void uart_on( USART_TypeDef* UARTx, int baud_rate, int rx_timeout );

// Send a string over UART (blocking)
void uart_tx_str( USART_TypeDef* UARTx,
                  const unsigned char* str, size_t len );

#endif
