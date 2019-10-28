/*
 * Minimal UART interface methods.
 */
#include "hal/uart.h"

// Enable a UART peripheral with a given baud rate
// and receive timeout, with asynchronous receives.
// RXNE/RTOF interrupts should be defined and NVIC interrupt enabled.
void uart_on( USART_TypeDef* UARTx, int baud_rate, int rx_timeout ) {
  // Set baud rate.
  UARTx->BRR   =  ( SystemCoreClock / baud_rate );
  // Set and enable RX timeout.
  // I think that this tells it to trigger a timeout after 10
  // 1-bit-long cycles have elapsed. So in this case, every 960Hz?
  UARTx->RTOR &= ~( USART_RTOR_RTO );
  UARTx->RTOR |=  ( rx_timeout << USART_RTOR_RTO_Pos );
  UARTx->CR2  |=  ( USART_CR2_RTOEN );
  // Enable the TX and RX lines, the RX interrupt,
  // the RX timeout interrupt, and the peripheral.
  UARTx->CR1  |=  ( USART_CR1_UE |
                    USART_CR1_RE |
                    USART_CR1_RXNEIE |
                    USART_CR1_RTOIE |
                    USART_CR1_TE );
}

// Send a string over UART (blocking)
void uart_tx_str( USART_TypeDef* UARTx,
                  const unsigned char* str, size_t len ) {
  while( len-- ) {
    while( !( UARTx->ISR & USART_ISR_TXE ) ) {};
    UARTx->TDR = *str++;
  }
}
