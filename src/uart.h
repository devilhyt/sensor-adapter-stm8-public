/**
 * @file uart.h
 * @author DevilHYT (devilhyt@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-02-10
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef UART_H
#define UART_H

#include "STM8S103F3.h"
#include <string.h>
#include <typedef.h>

/**
 * @brief UART TX status
 */
typedef enum {
  TxBusy = false,
  TxOk   = true,
} tx_status_t;

/**
 * @brief Disable UART1
 */
void uart_disable();

/**
 * @brief Setup UART1 pins
 */
void uart_pin_setup();

/**
 * @brief Initialize UART1
 *
 * @param baudrate Baudrate
 */
void uart_enable(const u16 baudrate);

/**
 * @brief Set UART1 baudrate
 *
 * @param baudrate Baudrate
 */
void uart_set_baudrate(const u32 baudrate);

/**
 * @brief Switch UART1 baudrate
 *
 * @param baudrate Baudrate
 * @return tx_status_t UART TX status
 * @retval TxBusy UART TX is busy
 * @retval TxOk UART TX is OK
 */
tx_status_t uart_switch_baudrate(const u32 baudrate);

/**
 * @brief Transmit message via UART1
 *
 * @param msg Message
 * @param msg_len Message length
 * @return tx_status_t UART TX status
 * @retval TxBusy UART TX is busy
 * @retval TxOk UART TX is OK
 */
tx_status_t uart_transmit(u8 *msg, u8 msg_len);

/**
 * @brief Check if UART1 is available
 *
 * @return bool UART availability
 * @retval true UART is available
 * @retval false UART is not available
 */
bool uart_available();

/**
 * @brief Read message from UART1
 *
 * @return u8 Message
 */
u8 uart_read();

/**
 * @brief UART1 TX interrupt handler
 */
ISR_HANDLER(IRQN_UART_TX_handler, _UART1_T_TC_VECTOR_);

/**
 * @brief UART1 RX interrupt handler
 */
ISR_HANDLER(IRQN_UART_RX_handler, _UART1_R_RXNE_VECTOR_);

#endif // UART_H