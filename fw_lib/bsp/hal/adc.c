/*
 * Minimal ADC interface methods.
 */
#include "hal/adc.h"

// Turn on the ADC and optionally calibrate it.
// Note: This does not enable the ADC peripheral.
void adc_on( ADC_TypeDef* ADCx, int perform_calibration ) {
  // Bring the ADC out of 'deep power-down' mode.
  ADCx->CR    &= ~( ADC_CR_DEEPPWD );
  // Enable the ADC voltage regulator.
  ADCx->CR    |=  ( ADC_CR_ADVREGEN );
  // Wait at least 20us (TODO: Use a timer or something).
  delay_cycles( 100 );
  // Calibrate the ADC if necessary.
  if ( perform_calibration ) {
    ADCx->CR  |=  ( ADC_CR_ADCAL );
    while ( ADCx->CR & ADC_CR_ADCAL ) {};
  }
}

// Configure the ADC to read a given set of channel(s) in sequence.
void adc_sequence_config( ADC_TypeDef* ADCx, adc_conversion* channels, int num_channels ) {
  if ( num_channels < 1 || num_channels > 16 ) { return; }
  // First, set the number of channels to read during each sequence.
  ADCx->SQR1 &= ~( ADC_SQR1_L );
  ADCx->SQR1 |=  ( ( num_channels - 1 ) << ADC_SQR1_L_Pos );
  // Next, configure each channel's position in the sequence and
  // their sampling times. TODO: Error checking on struct values.
  for ( int i = 1; i <= num_channels; ++i ) {
    if ( i < 5 ) {
      ADCx->SQR1  &= ~( 0x1F << ( 6 * i ) );
      ADCx->SQR1  |=  ( channels[ i - 1 ].channel << ( 6 * i ) );
    }
    else if ( i < 10 ) {
      ADCx->SQR2  &= ~( 0x1F << ( 6 * ( i - 5 ) ) );
      ADCx->SQR2  |=  ( channels[ i - 1 ].channel << ( 6 * ( i - 5 ) ) );
    }
    else if ( i < 15 ) {
      ADCx->SQR3  &= ~( 0x1F << ( 6 * ( i - 10 ) ) );
      ADCx->SQR3  |=  ( channels[ i - 1 ].channel << ( 6 * ( i - 10 ) ) );
    }
    else {
      ADCx->SQR4  &= ~( 0x1F << ( 6 * ( i - 15 ) ) );
      ADCx->SQR4  |=  ( channels[ i - 1 ].channel << ( 6 * ( i - 15 ) ) );
    }
    if ( channels[ i - 1 ].channel < 10 ) {
      ADCx->SMPR1 &= ~( 0x7 << ( channels[ i - 1 ].channel * 3 ) );
      ADCx->SMPR1 |=  ( channels[ i - 1 ].sample_time << ( channels[ i - 1 ].channel * 3 ) );
    }
    else {
      ADCx->SMPR2 &= ~( 0x7 << ( ( channels[ i - 1 ].channel - 10 ) * 3 ) );
      ADCx->SMPR2 |=  ( channels[ i - 1 ].sample_time << ( ( channels[ i - 1 ].channel - 10 ) * 3 ) );
    }
  }
}

// Perform a single ADC conversion.
// Note: The ADC must be configured to read only one channel before
// this method is called, and it should not be in continuous mode.
uint16_t adc_single_conversion( ADC_TypeDef* ADCx ) {
  // Start the ADC conversion.
  ADCx->CR  |=  ( ADC_CR_ADSTART );
  // Wait for the 'End Of Conversion' flag.
  while ( !( ADCx->ISR & ADC_ISR_EOC ) ) {};
  // Read the converted value (this also clears the EOC flag).
  uint16_t adc_val = ADCx->DR;
  // Wait for the 'End Of Sequence' flag and clear it.
  while ( !( ADCx->ISR & ADC_ISR_EOS ) ) {};
  ADCx->ISR |=  ( ADC_ISR_EOS );
  // Return the ADC value.
  return adc_val;
}
