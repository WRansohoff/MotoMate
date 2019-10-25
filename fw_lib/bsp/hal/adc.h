/*
 * Minimal ADC interface methods.
 */
#ifndef __VVC_ADC
#define __VVC_ADC

// HAL includes.
#include "hal/rcc.h"

// Device header file.
#include "stm32l4xx.h"

/* Preprocessor definitions */
// ADC clock source options.
#define ADC_CLOCK_NONE     ( 0 )
#define ADC_CLOCK_PLLSAI1R ( 1 )
#define ADC_CLOCK_PLLSAI2R ( 2 )
#define ADC_CLOCK_SYSCLK   ( 3 )
// ADC Sampling time values. The numbers in the names refer to
// how many ADC clock cycles the peripheral will take to sample the
// input, minus 0.5 cycles. Ex: 'ADC_SAMP_2_CYC' waits 2.5 cycles.
#define ADC_SAMP_2_CYC     ( 0 )
#define ADC_SAMP_6_CYC     ( 1 )
#define ADC_SAMP_12_CYC    ( 2 )
#define ADC_SAMP_24_CYC    ( 3 )
#define ADC_SAMP_47_CYC    ( 4 )
#define ADC_SAMP_92_CYC    ( 5 )
#define ADC_SAMP_247_CYC   ( 6 )
#define ADC_SAMP_640_CYC   ( 7 )

/* Helper Macros */
// Set the ADC peripheral's clock source.
// After reset, the default source is 'none' and the ADC won't work.
#define adc_clock_source( type ) \
  RCC->CCIPR &= ~( RCC_CCIPR_ADCSEL ); \
  RCC->CCIPR |=  ( type << RCC_CCIPR_ADCSEL_Pos );

/* Peripheral struct definitions */
// ADC conversion sequence configuration: this struct defines
// a single channel to convert. An array of up to 16 of these
// structs defines a sequence of ADC channels to convert.
typedef struct adc_conversion {
  int channel;
  int sample_time;
} adc_conversion;

// Turn on the ADC and optionally calibrate it.
// Note: This does not enable the ADC peripheral.
void adc_on( ADC_TypeDef* ADCx, int perform_calibration );

// Configure the ADC to read a given set of channel(s) in sequence.
void adc_sequence_config( ADC_TypeDef* ADCx, adc_conversion* channels, int num_channels );

// Perform a single ADC conversion.
// Note: The ADC must be configured to read only one channel before
// this method is called, and it should not be in continuous mode.
uint16_t adc_single_conversion( ADC_TypeDef* ADCx );

#endif
