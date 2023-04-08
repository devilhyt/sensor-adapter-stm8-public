/**
 * @file lump.c
 * @author DevilHYT (devilhyt@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-02-10
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "lump.h"
#include "lump_p.h"

void lump_sensor_init(const lump_sensor_t *sensor) {
  lump_sensor_ptr           = sensor;
  lump_sensor_state         = SensorReset;
  lump_hsk_state            = HskSync;
}

void lump_set_sensor_mode_init_callback(void (*lump_sensor_mode_init)()) {
  lump_sensor_mode_init_callback = lump_sensor_mode_init;
}

void lump_set_sensor_running_callback(void (*lump_sensor_running)()) {
  lump_sensor_running_callback = lump_sensor_running;
}

void lump_run() {
  u32 current_ms = millis();

  /* Limit the sensor frequency to below 1000 Hz */
  if (current_ms == lump_last_ms)
    return;
  lump_last_ms = current_ms;

  lump_transmit();
  lump_receive();
}

void lump_transmit() {
  memset(lump_tx_buf, 0, sizeof(lump_tx_buf));

  /* Sensor state machine */
  switch (lump_sensor_state) {
    /* Disable UART pins to allow the host to detect this sensor using autoid */
    case SensorReset:
      iwdg_refresh();
      uart_disable();
      uart_pin_setup();
      lump_event_ms     = lump_last_ms;
      lump_sensor_state = SensorWaitForAutoId;
      break;

    /* Wait until detection is done; then enable UART and start the handshake */
    case SensorWaitForAutoId:
      if (lump_last_ms - lump_event_ms > AUTOID_DELAY) {
        iwdg_refresh();
        uart_enable(LOWEST_BITRATE);
        lump_event_ms      = lump_last_ms;
        lump_sync_attempts = 0;
        lump_sensor_state  = SensorHandshake;
        lump_hsk_state     = HskSync;
      }
      break;

    /* Handshake */
    case SensorHandshake:
      /* Handshake state machine */
      switch (lump_hsk_state) {
        case HskSync:
          // hidden
          break;
        /* Wait for sync from host
         * This state is exited through from the command processing code in the receive() function.
         */
        case HskWaitForSync:
          if (lump_last_ms - lump_event_ms > DELAY_BETWEEN_SYNCS) {
            if (++lump_sync_attempts >= MAX_SYNC_ATTEMPTS)
              lump_sensor_state = SensorReset;
            else
              lump_hsk_state = HskSync;
          }
          break;

        case HskId:
          // hidden
          break;

        case HskModes:
          // hidden
          break;

        case HskSpeed:
          // hidden
          break;

        case HskModesName: {
          iwdg_refresh();
          lump_hsk_mode_ptr = &(lump_sensor_ptr->modes[lump_hsk_mode_index]);
          if (lump_hsk_mode_name_symbol(lump_hsk_mode_ptr->name, INFO_NAME) == TxOk)
            lump_hsk_state = HskModesRaw;
          break;
        }

        case HskModesRaw:
          if (lump_hsk_mode_value(lump_hsk_mode_ptr->raw, INFO_RAW) == TxOk)
            lump_hsk_state = HskModesPct;
          break;

        case HskModesPct:
          if (lump_hsk_mode_value(lump_hsk_mode_ptr->pct, INFO_PCT) == TxOk)
            lump_hsk_state = HskModesSi;
          break;

        case HskModesSi:
          if (lump_hsk_mode_value(lump_hsk_mode_ptr->si, INFO_SI) == TxOk)
            lump_hsk_state = HskModesSymbol;
          break;

        case HskModesSymbol: {
          if (lump_hsk_mode_name_symbol(lump_hsk_mode_ptr->symbol, INFO_SYMBOL) == TxOk)
            lump_hsk_state = HskModesFormat;
          break;
        }
        case HskModesFormat:
          // hidden
          break;

        case HskModesPause:
          if (lump_last_ms - lump_event_ms > INTERMODE_PAUSE) {
            if (++lump_hsk_mode_index < lump_sensor_ptr->mode_num) { /* equal to "lump_hsk_mode_index++ < lump_sensor_ptr->mode_num - 1" */
              lump_hsk_state = HskModesName;                     /* Send another mode */
            } else {
              lump_hsk_state = HskAck;
            }
          }
          break;

        case HskAck:
          // hidden
          break;

        /* Wait for ACK reply
         * This state is exited through from the command processing code in the lump_receive() function.
         */
        case HskWaitForAck:
          if (lump_last_ms - lump_event_ms > ACK_TIMEOUT)
            lump_sensor_state = SensorReset;
          break;

        /* switch to high baudrate & switch to data mode */
        case HskSwitchBaudRate:
          iwdg_refresh();
          if (uart_switch_baudrate(lump_sensor_ptr->speed) == TxOk) {
            lump_sensor_state = SensorModeInit;
          }
          break;

        default:
          while (true)
            ; /* Error occurred. Wait for watchdog reset the sensor. */
          break;
      }
      break;

    case SensorModeInit:
      if (lump_sensor_mode_init_callback)
        lump_sensor_mode_init_callback();
      lump_sensor_state = SensorRunning;
      break;

    case SensorRunning:
      if (lump_sensor_running_callback)
        lump_sensor_running_callback();
      break;

    case SensorNack:
      // hidden
      break;

    default:
      while (true)
        ; /* Error occurred. Wait for watchdog reset the sensor. */
      break;
  }
}

void lump_receive() {
  u8 byte_received   = 0;
  lump_receive_state = ReceiveGetByte;
  while (true) {
    switch (lump_receive_state) {
      case ReceiveGetByte: {
        if (!uart_available())
          return;
        byte_received = uart_read();
        if (lump_rx_len == 0)
          lump_receive_state = ReceiveCheckMsgType;
        else
          lump_receive_state = ReceiveDecode;
        break;
      }
      case ReceiveCheckMsgType: {
        // hidden
        break;
      }
      case ReceiveDecode: {
        lump_rx_buf[lump_rx_index++] = byte_received;
        if (lump_rx_index >= lump_rx_len) {
          lump_rx_len        = 0;
          lump_receive_state = ReceiveChecksum;
        } else {
          lump_rx_checksum ^= byte_received; /* Calc checksum */
          lump_receive_state = ReceiveGetByte;
        }
        break;
      }
      case ReceiveChecksum: {
        if (lump_rx_checksum == byte_received) {
          lump_receive_state = RecevieOperate;
        } else if (lump_sensor_state == SensorRunning) {
          lump_sensor_state = SensorNack;
          return;
        } else {
          lump_receive_state = ReceiveGetByte;
        }
        break;
      }
      case RecevieOperate: {
        // hidden
        return;
      }
      default:
        while (true)
          ; /* Error occurred. Wait for watchdog reset the sensor. */
        break;
    }
  }
}

static tx_status_t lump_hsk_mode_name_symbol(char *ptr, u8 info_type) {
  // hidden
}

static tx_status_t lump_hsk_mode_value(lump_value_t *ptr, u8 info_type) {
  // hidden
}

static void *lump_memcpy_be2le(void *dst, const void *src, size_t len) {
  char *d       = dst;
  const char *s = src;
  s += len - 1;
  while (len--)
    *d++ = *s--;
  return dst;
}

static u8 lump_msg_encode(u8 msg_bit, u8 msg_len, u8 cmd_bit) {
  return msg_bit | (lump_log2(msg_len) << MSGLEN_SHIFT) | cmd_bit;
}

static u8 lump_msg_checksum(u8 *dst, u8 msg_len) {
  u8 checksum = 0xff;
  for (u8 i = 0; i < msg_len; ++i)
    checksum ^= dst[i];
  return checksum;
}

static u8 lump_log2(u8 x) {
  switch (x) {
    case 1:
      return 0;
    case 2:
      return 1;
    case 4:
      return 2;
    case 8:
      return 3;
    case 16:
      return 4;
    case 32:
      return 5;
    default:
      return 0;
  }
}

static u8 lump_exp2(u8 x) {
  switch (x) {
    case 0:
      return 1;
    case 1:
      return 2;
    case 2:
      return 4;
    case 3:
      return 8;
    case 4:
      return 16;
    case 5:
      return 32;
    default:
      return 64;
  }
}

static u8 lump_ceil_power2(u8 x) {
  if (x == 1 || x == 2)
    return x;
  else if (x <= 4)
    return 4;
  else if (x <= 8)
    return 8;
  else if (x <= 16)
    return 16;
  else if (x <= 32)
    return 32;
  else
    return 0;
}

sensor_state_t lump_get_sensor_state() {
  return lump_sensor_state;
}

u8 lump_get_current_mode() {
  return lump_current_mode;
}

bool lump_get_nack_force_send() {
  return lump_nack_force_send;
}

tx_status_t lump_send_data8(u8 b) {
  return lump_send_data8_array(&b, 1);
}

tx_status_t lump_send_data8_array(u8 *b, u8 len) {
  // hidden
}

tx_status_t lump_send_data16(u16 s) {
  return lump_send_data8_array((u8 *)(&s), 2);
}

tx_status_t lump_send_data16_array(u16 *s, u8 len) {
  return lump_send_data8_array((u8 *)(s), len * 2);
}

tx_status_t lump_send_data32(u32 l) {
  return lump_send_data8_array((u8 *)(&l), 4);
}

tx_status_t lump_send_data32_array(u32 *l, u8 len) {
  return lump_send_data8_array((u8 *)(l), len * 4);
}