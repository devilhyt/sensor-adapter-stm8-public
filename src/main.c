/**
 * @file main.c
 * @author DevilHYT (devilhyt@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-02-10
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "STM8S103F3.h"
#include "adc1.h"
#include "iwdg.h"
#include "lump.h"
#include "timer4.h"
#include "uart.h"

/* sensor definition*/
const lump_value_t raw           = {.low = 0, .high = 1023};
const lump_value_t pct           = {.low = 0, .high = 100};
const lump_value_t si            = {.low = 0, .high = 1023};
const lump_value_t mode_raw_1    = {.low = 0, .high = 1};
const lump_value_t mode_pct_1    = {.low = 0, .high = 100};
const lump_value_t mode_si_1     = {.low = 0, .high = 1};
const lump_sensor_mode_t modes[] = {{.name      = "Analog",
                                     .symbol    = "raw",
                                     .data_sets = 1,
                                     .data_type = DATA16,
                                     .figures   = 5,
                                     .decimals  = 0,
                                     .raw       = &raw,
                                     .pct       = &pct,
                                     .si        = &si},
                                    {.name      = "Digital",
                                     .symbol    = "raw",
                                     .data_sets = 1,
                                     .data_type = DATA8,
                                     .figures   = 1,
                                     .decimals  = 0,
                                     .raw       = &mode_raw_1,
                                     .pct       = &mode_pct_1,
                                     .si        = &mode_si_1},
                                    {.name      = "Up Time",
                                     .symbol    = "sec",
                                     .data_sets = 1,
                                     .data_type = DATA32,
                                     .figures   = 5,
                                     .decimals  = 0,
                                     .raw       = &raw,
                                     .pct       = &pct,
                                     .si        = &si}};
const lump_sensor_t sensor       = {.type      = 83,
                                    .mode_num  = 3,
                                    .view      = VIEW_DEFAULT,
                                    .speed     = 57600,
                                    .with_xtal = true,
                                    .modes     = modes};

/* function prototypes */
void init();
void clk_init();
void sensor_mode_init();
void sensor_running();

/* functions */
void init() {
  clk_init();
  tim4_init();
  iwdg_init();
  lump_sensor_init(&sensor);
  lump_set_sensor_mode_init_callback(&sensor_mode_init);
  lump_set_sensor_running_callback(&sensor_running);
  ENABLE_INTERRUPTS();
}

void clk_init() {
  sfr_CLK.CKDIVR.byte = 0x00; /* f_hsi = f_master = f_cpu = 16Mhz */
}

void sensor_mode_init() {
  switch (lump_get_current_mode()) {
    case 0:
      adc1_init();
      break;
    case 1:
      adc1_deinit();
      break;
    default:
      break;
  }
}

void sensor_running() {
  static u16 new_value_0  = 0;
  static u16 last_value_0 = 0;
  static u8 new_value_1   = 0;
  static u8 last_value_1  = 0;
  static u32 new_value_2  = 0;
  static u32 last_value_2 = 0;
  static bool force_send  = false;
  static u32 last_millis  = 0;
  static u32 period       = 8; /* 125Hz */
  u32 current_millis      = millis();

  switch (lump_get_current_mode()) {
    case 0:
      new_value_0 = adc1_measure();
      if (new_value_0 != last_value_0) {
        force_send   = true;
        last_value_0 = new_value_0;
      }
      break;
    case 1:
      new_value_1 = sfr_PORTD.IDR.IDR3;
      if (new_value_1 != last_value_1) {
        force_send   = true;
        last_value_1 = new_value_1;
      }
      break;
    case 2:
      new_value_2 = millis() / 1000;
      if (new_value_2 != last_value_2) {
        force_send   = true;
        last_value_2 = new_value_2;
      }
      break;
    default:
      break;
  }

  if (lump_get_nack_force_send() || (force_send && (current_millis - last_millis > period))) {
    bool ret = false;
    switch (lump_get_current_mode()) {
      case 0:
        ret = lump_send_data16(new_value_0);
        break;
      case 1:
        ret = lump_send_data8(new_value_1);
        break;
      case 2:
        ret = lump_send_data32(new_value_2);
        break;
      default:
        break;
    }
    if (ret) {
      force_send  = false;
      last_millis = current_millis;
    }
  }
}

void main() {
  init();
  while (true) {
    lump_run();
  }
}