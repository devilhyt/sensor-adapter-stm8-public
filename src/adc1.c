/**
 * @file adc1.c
 * @author DevilHYT (devilhyt@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-02-10
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "adc1.h"

void adc1_init() {
  sfr_ADC1.CR1.ADON  = 0; // Disable ADC1
  sfr_ADC1.CR1.SPSEL = 2; // f_ADC = f_MASTER/4
  sfr_ADC1.CR2.ALIGN = 1; // Right alignment
  sfr_ADC1.CSR.CH    = 4; // Select channel 4
  // sfr_ADC1.TDRH.byte = 0xFF;
  // sfr_ADC1.TDRL.byte = 0xFF;
}

void adc1_deinit() {
  sfr_ADC1.CR1.ADON = 0; // Disable ADC1
}

u16 adc1_measure() {
  u16 result;

  sfr_ADC1.CSR.EOC  = 0; // clear HW "conversion ready" flag
  sfr_ADC1.CR1.ADON = 1; // start conversion
  sfr_ADC1.CR1.ADON = 1; // When the ADON bit is set for the first time,
                         // it wakes up the ADC from power down mode. To start conversion,
                         // set the ADON bit in the ADC_CR1 register with a second write instruction.
  while (!sfr_ADC1.CSR.EOC)
    ; // wait for conversion to complete

  result = (u16)sfr_ADC1.DRL.byte;
  result += ((u16)sfr_ADC1.DRH.byte) << 8;
  return result;
}