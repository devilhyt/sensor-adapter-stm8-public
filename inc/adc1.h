/**
 * @file adc1.h
 * @author DevilHYT (devilhyt@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-02-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef ADC1_H
#define ADC1_H

#include "STM8S103F3.h"
#include "typedef.h"

/**
 * @brief Initialize ADC1
 */
void adc1_init();

/**
 * @brief Deinitialize ADC1
 */
void adc1_deinit();

/**
 * @brief Measure ADC1
 * 
 * @return u16 ADC1 value
 */
u16 adc1_measure();

#endif /* ADC1_H */