/**
 * @file uart.c
 * @author DevilHYT (devilhyt@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-02-10
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "uart.h"
#include "uart_p.h"

void uart_disable() {
  sfr_UART1.CR1.UARTD = 1;
}

void uart_pin_setup() {
  sfr_PORTD.DDR.DDR6 = 0;
  sfr_PORTD.CR1.C16  = 0;
  sfr_PORTD.CR2.C26  = 0;
  sfr_PORTD.DDR.DDR5 = 1;
  sfr_PORTD.ODR.ODR5 = 0;
  sfr_PORTD.CR1.C15  = 1;
}

void uart_enable(const u16 baudrate) {
  uart_set_baudrate(baudrate);
  sfr_UART1.CR1.byte = 0x00; /* 8 Data bits, no parity bit */
  sfr_UART1.CR3.byte = 0x00; /* 1 Stop bit */
  sfr_UART1.CR2.byte = 0x00;
  sfr_UART1.CR2.TEN  = 1;
  sfr_UART1.CR2.REN  = 1;
  sfr_UART1.CR2.RIEN = 1;
}

ISR_HANDLER(IRQN_UART_TX_handler, _UART1_T_TC_VECTOR_) {
  if (uart_tx_buf_index < uart_tx_buf_len) {
    sfr_UART1.DR.byte = uart_tx_buf[uart_tx_buf_index];
    ++uart_tx_buf_index;
  } else {
    sfr_UART1.CR2.TIEN = 0; /* disable tx interrupt */
    uart_tx_active     = false;
  }
}

ISR_HANDLER(IRQN_UART_RX_handler, _UART1_R_RXNE_VECTOR_) {
  uart_rx_buf[uart_rx_buf_head++] = sfr_UART1.DR.byte;
  uart_rx_buf_head %= UART_RX_BUF_SIZE; /* circular */
}

tx_status_t uart_switch_baudrate(const u32 baudrate) {
  if (uart_tx_active)
    return TxBusy;
  uart_set_baudrate(baudrate);
  return TxOk;
}

void uart_set_baudrate(const u32 baudrate) {
  u16 uart_div        = (u16)(((u32)16000000L) / baudrate);
  sfr_UART1.BRR2.byte = (u8)(((uart_div & 0xF000) >> 8) | (uart_div & 0x000F));
  sfr_UART1.BRR1.byte = (u8)((uart_div & 0x0FF0) >> 4);
}

tx_status_t uart_transmit(u8 *msg, u8 msg_len) {
  if (uart_tx_active)
    return TxBusy;

  memcpy(uart_tx_buf, msg, msg_len);

  /* Enable uart tx interrupt */
  uart_tx_buf_index  = 0;
  uart_tx_buf_len    = msg_len;
  sfr_UART1.CR2.TIEN = 1;
  uart_tx_active     = true;

  return TxOk;
}

u8 uart_read() {
  if (!uart_available())
    return 255; /* no data (-1)*/
  u8 data = uart_rx_buf[uart_rx_buf_tail++];
  uart_rx_buf_tail %= UART_RX_BUF_SIZE; /* circular */
  return data;
}

bool uart_available() {
  return (uart_rx_buf_head != uart_rx_buf_tail);
}