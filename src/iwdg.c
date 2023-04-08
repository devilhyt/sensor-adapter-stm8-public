/**
 * @file iwdg.c
 * @author DevilHYT (devilhyt@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-02-10
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "iwdg.h"

void iwdg_init() {
  sfr_IWDG.KR.byte  = KEY_ENABLE;
  sfr_IWDG.KR.byte  = KEY_ACCESS; /* Enable the access to the protected IWDG_PR and IWDG_RLR registers */
  sfr_IWDG.PR.byte  = 0x06;
  sfr_IWDG.RLR.byte = 0xFF;
  sfr_IWDG.KR.byte  = KEY_ENABLE;
}

void iwdg_refresh() {
  sfr_IWDG.KR.byte = KEY_REFRESH; /* Refresh watchdog */
}