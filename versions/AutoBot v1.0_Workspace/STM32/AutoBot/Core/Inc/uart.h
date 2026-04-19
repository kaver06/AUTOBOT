/*
 * uart.h
 *
 *  Created on: Nov 5, 2025
 *      Author: kaver
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>

#define RX_BUFFER_SIZE 256

extern UART_HandleTypeDef huart1;

// RX buffer declarations
extern uint8_t uart1_rx_buffer[RX_BUFFER_SIZE];
extern uint8_t uart1_data[RX_BUFFER_SIZE];
extern volatile uint8_t data_received_flag;

// Parsed ESP32-CAM data accessible globally
extern volatile int esp32_tag_id;
extern volatile float esp32_yaw;
extern volatile float esp32_pitch;
extern volatile float esp32_roll;
extern volatile float esp32_x;
extern volatile float esp32_y;
extern volatile float esp32_z;
extern volatile uint8_t new_esp32_data_flag;

// UART functions
void UART1_Init(void);
void UART1_SendString(const char *str);
void UART1_StartReception(void);
void UART1_ProcessReceivedData(void);

#endif /* INC_UART_H_ */
