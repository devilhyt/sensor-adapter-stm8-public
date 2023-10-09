/**
 * @file lump.h
 * @author DevilHYT (devilhyt@gmail.com)
 * @brief LEGO UART Message Protocol
 * @version 0.1
 * @date 2022-02-10
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef LUMP_H
#define LUMP_H

#include "iwdg.h"
#include "timer4.h"
#include "uart.h"
#include <typedef.h>

/* Message type */
#define MSG_SYS  0x00
#define MSG_CMD  0x40
#define MSG_INFO 0x80
#define MSG_DATA 0xC0

/* System message type (MSG_SYS | SYS_SYNC) */
#define SYS_SYNC 0x00
#define SYS_NACK 0x02
#define SYS_ACK  0x04
#define SYS_ESC  0x06

/* Command message type */
#define CMD_TYPE     0x00
#define CMD_MODES    0x01
#define CMD_SPEED    0x02
#define CMD_SELECT   0x03
#define CMD_WRITE    0x04
#define CMD_UNK1     0x05 /* Powered Up only */
#define CMD_EXT_MODE 0x06
#define CMD_VERSION  0x07

/* Info message type */
#define INFO_NAME        0x00
#define INFO_RAW         0x01
#define INFO_PCT         0x02
#define INFO_SI          0x03
#define INFO_SYMBOL      0x04
#define INFO_MAPPING     0x05 /* Powered Up only */
#define INFO_MODE_COMBOS 0x06 /* Powered Up only */
#define INFO_UNK7        0x07 /* Powered Up only */
#define INFO_UNK8        0x08 /* Powered Up only */
#define INFO_UNK9        0x09 /* Powered Up only */
#define INFO_UNK10       0x0A /* Powered Up only */
#define INFO_UNK11       0x0B /* Powered Up only */
#define INFO_UNK12       0x0C /* Powered Up only */
#define INFO_MODE_PLUS_8 0x20
#define INFO_FORMAT      0x80

/* Encoded message payload bytes */
#define MSG_LEN_1  0x00
#define MSG_LEN_2  0x08
#define MSG_LEN_4  0x10
#define MSG_LEN_8  0x18
#define MSG_LEN_16 0x20
#define MSG_LEN_32 0x28

/* Number of modes in view and data log */
#define VIEW_1       0x01 /* Only mode 0 */
#define VIEW_2       0x02 /* Mode 0,1 */
#define VIEW_3       0x03 /* Mode 0,1,2 */
#define VIEW_4       0x04 /* Mode 0,1,2,3 */
#define VIEW_5       0x05 /* Mode 0,1,2,3,4 */
#define VIEW_6       0x06 /* Mode 0,1,2,3,4,5 */
#define VIEW_7       0x07 /* Mode 0,1,2,3,4,5,6 */
#define VIEW_8       0x08 /* Mode 0,1,2,3,4,5,6,7 */
#define VIEW_DEFAULT 0xFF /* All modes */

/* Bit mask */
#define MSG_MASK       0xC0
#define SYS_MASK       0x07
#define CMD_MASK       0x07
#define INFO_MODE_MASK 0x07
#define MSGLEN_MASK    0x38

/* Bit Shift */
#define MSGLEN_SHIFT 3

/* Data set format */
#define DATA8  0x00
#define DATA16 0x01
#define DATA32 0x02
#define DATAF  0x03

/* Timeout threshold */
#define AUTOID_DELAY        500 /* ms */
#define DELAY_BETWEEN_SYNCS 6   /* ms */
#define MAX_SYNC_ATTEMPTS   10
#define INTERMODE_PAUSE     30 /* ms */
#define ACK_TIMEOUT         80 /* ms */

/* MISC */
#define LOWEST_BITRATE 2400
#define MAX_MODES      8
#define MAX_MSG_LEN    32
#define BUF_SIZE       35
#define DEFAULT_VALUE  NULL

/* Sensor state */
typedef enum {
  SensorReset,
  SensorWaitForAutoId,
  SensorHandshake,
  SensorModeInit,
  SensorRunning,
  SensorNack,
} sensor_state_t;

/* Handshake state */
typedef enum {
  HskSync,
  HskWaitForSync,
  HskId,
  HskModes,
  HskSpeed,
  HskModesName,
  HskModesRaw,
  HskModesPct,
  HskModesSi,
  HskModesSymbol,
  HskModesFormat,
  HskModesPause,
  HskAck,
  HskWaitForAck,
  HskSwitchBaudRate,
} hsk_state_t;

/* Receive state */
typedef enum {
  ReceiveGetByte,
  ReceiveCheckMsgType,
  ReceiveDecode,
  ReceiveChecksum,
  RecevieOperate,
} receive_state_t;

/**
 * Sensor structure
 */

/* LUMP Value */
typedef struct {
    f32 low;
    f32 high;
} lump_value_t;

/* LUMP Mode */
typedef struct {
    char name[12];
    char symbol[5];
    u8 data_sets;
    u8 data_type;
    u8 figures;
    u8 decimals;
    lump_value_t *raw;
    lump_value_t *pct;
    lump_value_t *si;
} lump_sensor_mode_t;

/* LUMP Sensor */
typedef struct {
    u8 type;
    u8 mode_num;
    u8 view;
    u32 speed;
    bool with_xtal;
    lump_sensor_mode_t *modes;
} lump_sensor_t;

/* function prototypes */

/**
 * @brief Initialize lump sensor
 *
 * @param sensor sensor information
 */
void lump_sensor_init(const lump_sensor_t *sensor);

/**
 * @brief
 *
 * @param lump_sensor_mode_init callback function for mode initialization
 */
void lump_set_sensor_mode_init_callback(void (*lump_sensor_mode_init)());

/**
 * @brief
 *
 * @param lump_sensor_running callback function for sensor running
 */
void lump_set_sensor_running_callback(void (*lump_sensor_running)());

/**
 * @brief  Run lump sensor
 */
void lump_run();

/**
 * @brief  Transmit
 */
void lump_transmit();

/**
 * @brief  Receive
 */
void lump_receive();

/**
 * @brief  Get sensor state
 *
 * @return sensor_state_t sensor state
 */
sensor_state_t lump_get_sensor_state();

/**
 * @brief Get current mode
 *
 * @return current mode
 */
u8 lump_get_current_mode();

/**
 * @brief Get nack force send flag
 *
 * @return  nack force send flag
 */
bool lump_get_nack_force_send();

/**
 * @brief send data8
 *
 * @param b data
 * @return tx_status_t
 * @retval TxOk transmission successful
 * @retval TXBusy transmission busy
 */
tx_status_t lump_send_data8(u8 b);

/**
 * @brief send data8 array
 *
 * @param b data array
 * @param len data array length
 * @return tx_status_t
 * @retval TxOk transmission successful
 * @retval TXBusy transmission busy
 */
tx_status_t lump_send_data8_array(u8 *b, u8 len);

/**
 * @brief send data16
 *
 * @param s data
 * @return tx_status_t
 * @retval TxOk transmission successful
 * @retval TXBusy transmission busy
 */
tx_status_t lump_send_data16(u16 s);

/**
 * @brief send data16 array
 *
 * @param s data array
 * @param len data array length
 * @return tx_status_t
 * @retval TxOk transmission successful
 * @retval TXBusy transmission busy
 */
tx_status_t lump_send_data16_array(u16 *s, u8 len);

/**
 * @brief send data32
 *
 * @param l data
 * @return tx_status_t
 * @retval TxOk transmission successful
 * @retval TXBusy transmission busy
 */
tx_status_t lump_send_data32(u32 l);

/**
 * @brief send data32 array
 *
 * @param l data array
 * @param len data array length
 * @return tx_status_t
 * @retval TxOk transmission successful
 * @retval TXBusy transmission busy
 */
tx_status_t lump_send_data32_array(u32 *l, u8 len);

#endif /* LUMP_H */