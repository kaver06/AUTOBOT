/*
 * ir_array.h
 *
 *  Created on: Nov 2, 2025
 *      Author: kaver
 */

#ifndef INC_IR_ARRAY_H_
#define INC_IR_ARRAY_H_

#include "stm32f4xx_hal.h"

#define IR_COUNT	4
#define ESP_CAM_PORT	GPIOF
#define ESP_CAM_PIN		GPIO_PIN_9
typedef struct{
	uint8_t state[IR_COUNT];
}IR_Array_T;

void IR_Array_Read(IR_Array_T *ir_data);
void IR_Decision(IR_Array_T *ir_data);
uint8_t IR_Array_GetPattern(IR_Array_T *ir_data);
#endif /* INC_IR_ARRAY_H_ */
