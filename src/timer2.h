/**
 * @file timer2.h
 * @author DevilHYT (devilhyt@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-02-10
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef TIMER2_H
#define TIMER2_H

#include "STM8S103F3.h"
#include <typedef.h>

/**
 * @brief Initialize timer2
 */
void tim2_init();

/**
 * @brief Timer2 interrupt handler
 */
ISR_HANDLER(IRQN_TIM2_handler, _TIM2_OVR_UIF_VECTOR_);

#endif // TIMER2_H