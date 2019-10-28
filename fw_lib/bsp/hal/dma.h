/*
 * Minimal DMA interface methods.
 */
#ifndef __VVC_DMA
#define __VVC_DMA

// Device header file.
#include "stm32l4xx.h"

// Preprocessor definitions for DMA configuration options.
// DMA channel priority:
#define DMA_PRIORITY_LOW ( 0 )
#define DMA_PRIORITY_MID ( 1 )
#define DMA_PRIORITY_HI  ( 2 )
#define DMA_PRIORITY_VHI ( 3 )
// Transfer 'word' sizes.
#define DMA_SIZE_8b      ( 0 )
#define DMA_SIZE_16b     ( 1 )
#define DMA_SIZE_32b     ( 2 )

// Configure a DMA channel for memory-to-peripheral
// transfers, with given sizes/priority/addresses/counts.
void dma_config_tx( uint32_t dmax, int chan, uint32_t data_src,
                    uint32_t data_dst, uint16_t data_len,
                    int periph, int priority,
                    int word_size, int circ, int tcie );

#endif
