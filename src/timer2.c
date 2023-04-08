/**
 * @file timer2.c
 * @author DevilHYT (devilhyt@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-02-10
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "timer2.h"

void tim2_init(void) {
  // TIM2_EGR   = 0x01;                  /* Re-initializes */
  // TIM2_IER   = 0x01;                  /* Update interrupt enable */
  // TIM2_ARRH  = (uint8_t)(16000 >> 8); /* 16000 */
  // TIM2_ARRL  = (uint8_t)(16000);
  // TIM2_PSCR  = 0x00;
  // TIM2_CR1   = 0x01; /* Enable TIM2 */
}

ISR_HANDLER(IRQN_TIM2_handler, _TIM2_OVR_UIF_VECTOR_) {
  // bitClr(TIM2_SR1, UIF);
}