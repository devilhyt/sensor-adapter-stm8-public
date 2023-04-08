/**
 * @file lump_p.h
 * @author DevilHYT (devilhyt@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-02-10
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef LUMP_P_H
#define LUMP_P_H

#include "uart.h"
#include <typedef.h>

#define LUMP_TX_BUF_SIZE 35
#define LUMP_RX_BUF_SIZE 35

////////////////////////// Private variables //////////////////////////
static const lump_sensor_t *lump_sensor_ptr = NULL; /* Sensor pointer */

/* Sensor mode */
static const lump_sensor_mode_t *lump_hsk_mode_ptr = NULL; /* mode pointer for handshake */
static u8 lump_hsk_mode_index                      = 0;    /* mode index for handshake */
static u8 lump_current_mode                        = 0;    /* sensor current mode */

/* timing */
static volatile u32 lump_last_ms  = 0; /* Millisecond handled in the last sensor loop */
static volatile u32 lump_event_ms = 0; /* Auxiliary ms counter for timing handshake parts */

/* State machine */
static u8 lump_sensor_state;  /* sensor state */
static u8 lump_hsk_state;     /* handshake state */
static u8 lump_receive_state; /* receive state */

/* LUMP TX */
static u8 lump_tx_buf[LUMP_TX_BUF_SIZE] = {0};

/* LUMP RX */
static u8 lump_rx_buf[LUMP_RX_BUF_SIZE] = {0};
static u8 lump_rx_len                   = 0;
static u8 lump_rx_index                 = 0;
static u8 lump_rx_checksum              = 0xFF;

/* Callback functions */
static void (*lump_sensor_mode_init_callback)() = NULL; /* Called when the sensor mode is changed */
static void (*lump_sensor_running_callback)()   = NULL; /* called when the sensor is running */

/* MISC */
static bool lump_nack_force_send = false;
static u8 lump_sync_attempts     = 0; /* How many times the sync byte (sensor handshake) was sent */

////////////////////////// Private functions //////////////////////////
/**
 * @brief Utility function to send a mode's name or mode's symbol
 *
 * @param ptr Pointer to the name string or symbol string
 * @param info_type Info type (INFO_NAME, INFO_SYMBOL)
 * @return tx_status_t
 * @retval TxBusy UART TX is busy
 * @retval TxOk UART TX is OK
 */
static tx_status_t lump_hsk_mode_name_symbol(char *ptr, u8 info_type);

/**
 * @brief Utility function to send a mode's value span
 *
 * @param ptr pointer to the char array
 * @param info_type Pointer to the value span(lump_value_t)
 * @return tx_status_t
 * @retval TxBusy UART TX is busy
 * @retval TxOk UART TX is OK
 */
static tx_status_t lump_hsk_mode_value(lump_value_t *ptr, u8 info_type);

/**
 * @brief memcpy() with big-endian to little-endian conversion.
 *
 * @param dst address of destination
 * @param src address of source
 * @param len bytes to copy
 * @return address of destination
 */
static void *lump_memcpy_be2le(void *dst, const void *src, size_t len);

/**
 * @brief Encode message type, length and command
 *
 * @param msg_bit message type
 * @param msg_len message length
 * @param cmd_bit command type
 * @return encoded message
 */
static u8 lump_msg_encode(u8 msg_bit, u8 msg_len, u8 cmd_bit);

/**
 * @brief Calculate checksum of a message
 *
 * @param dst pointer to the message
 * @param msg_len length of the message
 * @return checksum
 */
static u8 lump_msg_checksum(u8 *dst, u8 msg_len);

/**
 * @brief Utility method to compute binary logarithm (up to 32)
 *
 * @param x Value to compute the binary logarithm
 * @return x's binary logarithm
 */
static u8 lump_log2(u8 x);

/**
 * @brief Utility method to compute power of 2 (up to 5)
 *
 * @param x Value to compute the power of 2
 * @return x power of 2
 */
static u8 lump_exp2(u8 x);

/**
 * @brief Utility method to compute next power of 2 (up to 32)
 *
 * @param x Value to compute the next power of 2
 * @return x's next power of 2
 */
static u8 lump_ceil_power2(u8 x);

#endif // LUMP_P_H