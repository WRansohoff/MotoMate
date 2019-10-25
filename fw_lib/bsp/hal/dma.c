/*
 * Minimal DMA interface methods.
 */
#include "hal/dma.h"

// Configure a DMA channel for one-shot memory-to-peripheral
// transfers, with given sizes/priority/addresses/counts.
void dma_config_tx_single( uint32_t dmax, int chan, uint32_t data_src,
                           uint32_t data_dst, uint16_t data_len,
                           int periph, int priority,
                           int word_size, int tcie ) {
  // Calculate DMA struct memory addresses from DMAx and Channel #.
  DMA_Channel_TypeDef* DMA_CHx = ( DMA_Channel_TypeDef* )( dmax + 0x08 + ( 0x14 * ( chan - 1 ) ) );
  DMA_Request_TypeDef* DMA_RQx = ( DMA_Request_TypeDef* )( dmax + 0xA8 );
  // Configure DMA channel settings.
  // * Memory-to-peripheral.
  // * Circular mode disabled.
  // * Increment memory pointer, don't increment periph ptr.
  // * Data size depends on input (0 = 8b, 1 = 16b, 2 = 32b).
  // * Priority depends on input.
  // * Enable 'transfer complete' interrupt if 'tcie' input != 0.
  DMA_CHx->CCR &= ~( DMA_CCR_MEM2MEM |
                     DMA_CCR_PL |
                     DMA_CCR_CIRC |
                     DMA_CCR_MSIZE |
                     DMA_CCR_PSIZE |
                     DMA_CCR_PINC |
                     DMA_CCR_EN );
  DMA_CHx->CCR |=  ( ( priority << DMA_CCR_PL_Pos ) |
                     ( word_size << DMA_CCR_MSIZE_Pos ) |
                     ( word_size << DMA_CCR_PSIZE_Pos ) |
                     DMA_CCR_MINC |
                     ( !!tcie ) << DMA_CCR_TCIE_Pos |
                     DMA_CCR_DIR );
  // Select the appropriate peripheral in the shared 'CSELR' register.
  int cselr_offset = ( chan - 1 ) * 4;
  DMA_RQx->CSELR &= ~( 0xF << cselr_offset );
  DMA_RQx->CSELR |=  ( periph << cselr_offset );
  // Set DMA source/destination addresses, and the transfer length.
  // Source: Starting address of the memory to transfer.
  DMA_CHx->CMAR   = data_src;
  // Destination: Address of where the data should be sent.
  DMA_CHx->CPAR   = data_dst;
  // Length: Number of 'words' (size depends on input) to transfer.
  DMA_CHx->CNDTR  = data_len;
}
