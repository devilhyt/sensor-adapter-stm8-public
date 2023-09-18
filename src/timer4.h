/**
 * @file timer4.h
 * @author DevilHYT (devilhyt@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-02-10
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef TIMER4_H
#define TIMER4_H

#include "STM8S103F3.h"
#include <typedef.h>

/**
 * @brief Initialize TIM4
 */
void tim4_init();

/**
 * @brief Get current millisecond(ms)
 *
 * @return u32 Millisecond(ms)
 */
u32 millis();

/**
 * @brief TIM4 overflow interrupt handler
 */
ISR_HANDLER(IRQN_TIM4_handler, _TIM4_OVR_UIF_VECTOR_);

#endif // TIMER4_H