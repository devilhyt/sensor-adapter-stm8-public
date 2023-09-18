/**
 * @file iwdg.h
 * @author DevilHYT (devilhyt@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-02-10
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef IWDG_H
#define IWDG_H

#include "STM8S103F3.h"
#include <typedef.h>

/* IWDG register key value */
#define KEY_ENABLE  0xCC
#define KEY_ACCESS  0x55
#define KEY_REFRESH 0xAA

/**
 * @brief Initialize IWDG
 */
void iwdg_init();

/**
 * @brief Refresh IWDG
 */
void iwdg_refresh();

#endif // IWDG_H