/**
 * @file timer4.c
 * @author DevilHYT (devilhyt@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-02-10
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "timer4.h"
#include "timer4_p.h"

void tim4_init() {
  sfr_TIM4.EGR.byte  = 0x01; /* Re-initializes */
  sfr_TIM4.IER.byte  = 0x01; /* Update interrupt enable */
  sfr_TIM4.ARR.byte  = 250;  /* 250 times */
  sfr_TIM4.PSCR.byte = 0x06; /* 16Mhz / (2^6) = 250Khz */
  sfr_TIM4.CR1.byte  = 0x01; /* Enable TIM4 */
}

u32 millis() {
  return tim4_cnt;
}

ISR_HANDLER(IRQN_TIM4_handler, _TIM4_OVR_UIF_VECTOR_) {
  ++tim4_cnt;
  // ++led_cnt;
  // if (led_cnt >= 1000) {
  //   sfr_PORTB.ODR.ODR5 ^= 1;
  //   led_cnt = 0;
  // }
  sfr_TIM4.SR.UIF = 0;
}