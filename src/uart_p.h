/**
 * @file uart_p.h
 * @author DevilHYT (devilhyt@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-02-10
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef UART_P_H
#define UART_P_H

#include <typedef.h>

/* UART buffer size */
#define UART_TX_BUF_SIZE 35
#define UART_RX_BUF_SIZE 35

/* Variables for UART TX interrupt (sensor->host) */
static volatile u8 uart_tx_buf[UART_TX_BUF_SIZE] = {0};   /* message buffer for UART interrupt */
static volatile u8 uart_tx_buf_len               = 0;     /* how many bytes there are yet for the UART write interrupt to transmit */
static volatile u8 uart_tx_buf_index             = 0;     /* index of tx buffer */
static volatile bool uart_tx_active              = false; /* hether UART transmission is in progress */

/* Variables for UART RX interrupt (host->sensor) */
static volatile u8 uart_rx_buf[UART_RX_BUF_SIZE] = {0}; /* message buffer for UART interrupt */
static volatile u8 uart_rx_buf_head              = 0;   /* head index in the RX circular buffer */
static u8 uart_rx_buf_tail                       = 0;   /* tail index in the RX circular buffer */

#endif // UART_P_H