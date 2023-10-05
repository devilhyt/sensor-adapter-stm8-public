/**
 * @file lump.c
 * @author DevilHYT (devilhyt@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-02-10
 * 
 * @copyright Copyright (c) 2022
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
          lump_tx_buf[0] = SYS_SYNC;
          if (uart_transmit(lump_tx_buf, 1) == TxOk) {
            if (lump_sensor_ptr->with_xtal) {
              lump_hsk_state = HskId;
            } else {
              lump_event_ms  = lump_last_ms;
              lump_hsk_state = HskWaitForSync;
            }
          }
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
          lump_tx_buf[0] = lump_msg_encode(MSG_CMD, 1, CMD_TYPE);
          lump_tx_buf[1] = lump_sensor_ptr->type;
          lump_tx_buf[2] = lump_msg_checksum(lump_tx_buf, 2);
          if (uart_transmit(lump_tx_buf, 3) == TxOk)
            lump_hsk_state = HskModes;
          break;

        case HskModes:
          lump_tx_buf[1] = lump_sensor_ptr->mode_num - 1;
          if (lump_sensor_ptr->view == VIEW_DEFAULT) {
            lump_tx_buf[0] = lump_msg_encode(MSG_CMD, 1, CMD_MODES);
            lump_tx_buf[2] = lump_msg_checksum(lump_tx_buf, 2);
            if (uart_transmit(lump_tx_buf, 3) == TxOk)
              lump_hsk_state = HskSpeed;
          } else {
            lump_tx_buf[0] = lump_msg_encode(MSG_CMD, 2, CMD_MODES);
            lump_tx_buf[2] = lump_sensor_ptr->view - 1;
            lump_tx_buf[3] = lump_msg_checksum(lump_tx_buf, 3);
            if (uart_transmit(lump_tx_buf, 4) == TxOk)
              lump_hsk_state = HskSpeed;
          }
          break;

        case HskSpeed:
          lump_tx_buf[0] = lump_msg_encode(MSG_CMD, 4, CMD_SPEED);
          lump_memcpy_be2le(&lump_tx_buf[1], &(lump_sensor_ptr->speed), 4);
          lump_tx_buf[5] = lump_msg_checksum(lump_tx_buf, 5);
          if (uart_transmit(lump_tx_buf, 6) == TxOk) {
            lump_hsk_mode_ptr   = NULL;
            lump_hsk_mode_index = 0;
            lump_hsk_state  = HskModesName;
          }
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
          lump_tx_buf[0] = lump_msg_encode(MSG_INFO, 4, lump_hsk_mode_index);
          lump_tx_buf[1] = INFO_FORMAT;
          lump_tx_buf[2] = lump_hsk_mode_ptr->data_sets;
          lump_tx_buf[3] = lump_hsk_mode_ptr->data_type;
          lump_tx_buf[4] = lump_hsk_mode_ptr->figures;
          lump_tx_buf[5] = lump_hsk_mode_ptr->decimals;
          lump_tx_buf[6] = lump_msg_checksum(lump_tx_buf, 6);
          if (uart_transmit(lump_tx_buf, 7) == TxOk) {
            lump_event_ms  = lump_last_ms;
            lump_hsk_state = HskModesPause;
          }
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
          lump_tx_buf[0] = SYS_ACK;
          if (uart_transmit(lump_tx_buf, 1) == TxOk) {
            lump_event_ms  = lump_last_ms;
            lump_hsk_state = HskWaitForAck;
          }
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
      lump_tx_buf[0] = SYS_NACK;
      while (uart_transmit(lump_tx_buf, 1) == TxBusy)
        ;
      lump_sensor_state = SensorRunning;
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
        if (uart_rx_buf_is_empty())
          return;
        byte_received = uart_read();
        if (lump_rx_len == 0)
          lump_receive_state = ReceiveCheckMsgType;
        else
          lump_receive_state = ReceiveDecode;
        break;
      }
      case ReceiveCheckMsgType: {
        if (byte_received == SYS_SYNC || byte_received == SYS_NACK || byte_received == SYS_ACK) { /* New system message */
          lump_rx_buf[0]     = byte_received;
          lump_receive_state = RecevieOperate;
        } else { /* Other types of messages */
          lump_rx_len = lump_exp2((byte_received & MSGLEN_MASK) >> MSGLEN_SHIFT) + 2;
          if (lump_rx_len <= MAX_MSG_LEN) {
            lump_rx_index      = 0;
            lump_rx_checksum   = 0xFF;
            lump_receive_state = ReceiveDecode;
          } else { /* Payload length too long. look for next message */
            lump_rx_len        = 0;
            lump_receive_state = ReceiveGetByte;
          }
        }
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
        if (lump_rx_buf[0] == SYS_NACK) {
          iwdg_refresh();
          lump_nack_force_send = true;
          return;
        }

        switch (lump_sensor_state) {
          /* Handle host messages in handshake */
          case SensorHandshake: {
            if (lump_hsk_state == HskWaitForSync && lump_rx_buf[0] == SYS_SYNC)
              lump_hsk_state = HskId; /* Sync sucess, start handshake */
            if (lump_hsk_state == HskWaitForAck && lump_rx_buf[0] == SYS_ACK)
              lump_hsk_state = HskSwitchBaudRate; /* Ack received, start data mode */
            break;
          }
          /* Handle host messages in running condition */
          case SensorRunning: {
            if (lump_rx_buf[0] == (MSG_CMD | CMD_SELECT)) { /* Select mode */
              lump_current_mode = lump_rx_buf[1];
              lump_sensor_state = SensorModeInit;
            }
            break;
          }
          default:
            break;
        }
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
  u8 len = strlen(ptr);
  memcpy(&lump_tx_buf[2], ptr, len); // No zero termination necessary
  len                  = lump_ceil_power2(len);
  lump_tx_buf[0]       = lump_msg_encode(MSG_INFO, len, lump_hsk_mode_index);
  lump_tx_buf[1]       = info_type;
  lump_tx_buf[len + 2] = lump_msg_checksum(lump_tx_buf, len + 2);
  return uart_transmit(lump_tx_buf, len + 3);
}

static tx_status_t lump_hsk_mode_value(lump_value_t *ptr, u8 info_type) {
  if (ptr) {
    lump_tx_buf[0] = lump_msg_encode(MSG_INFO, 8, lump_hsk_mode_index);
    lump_tx_buf[1] = info_type;
    lump_memcpy_be2le(&lump_tx_buf[2], &(ptr->low), 4);
    lump_memcpy_be2le(&lump_tx_buf[6], &(ptr->high), 4);
    lump_tx_buf[10] = lump_msg_checksum(lump_tx_buf, 10);
    return uart_transmit(lump_tx_buf, 11);
  }
  return TxOk;
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
  lump_tx_buf[0] = lump_msg_encode(MSG_DATA, len, lump_current_mode);
  lump_memcpy_be2le(&lump_tx_buf[1], b, len);
  lump_tx_buf[len + 1] = lump_msg_checksum(lump_tx_buf, len + 1);
  tx_status_t status   = uart_transmit(lump_tx_buf, len + 2);
  if (status)
    lump_nack_force_send = false;
  return status;
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