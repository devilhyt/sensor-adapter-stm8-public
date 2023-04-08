/**
 * @file timer4_p.h
 * @author DevilHYT (devilhyt@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-02-10
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef TIMER4_P_H
#define TIMER4_P_H

#include <typedef.h>

static volatile u32 tim4_cnt = 0; /* Millisecond(ms) counter, incremented by Tim4 interrupt */
// static volatile u16 led_cnt = 0; /* led blink counter */

#endif // TIMER4_P_H