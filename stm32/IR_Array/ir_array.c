/*
 * ir_array.c
 *
 *  Created on: Sep 18, 2025
 *      Author: kaver
 */

#include "ir_array.h"
#include "motor_control.h"

/*
 * left ir PA0
 * mid left PA1
 * mid right PA4
 * right ir PB0
 */

// Assign pins and ports for 4 sensors
const uint16_t ir_pins[IR_COUNT]  = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_4, GPIO_PIN_0};
GPIO_TypeDef* ir_ports[IR_COUNT]  = {GPIOA, GPIOA, GPIOA, GPIOB};

void IR_Array_Read(IR_Array_T *ir_data)
{
    for (int i = 0; i < IR_COUNT; i++)
    {
        ir_data->state[i] = HAL_GPIO_ReadPin(ir_ports[i], ir_pins[i]);
    }
}

// Convert 4 bits into one number
uint8_t IR_Array_GetPattern(IR_Array_T *ir_data)
{
    uint8_t pattern = 0;
    for (int i = 0; i < IR_COUNT; i++)
    {
        pattern <<= 1;
        pattern |= (ir_data->state[i] & 0x01);
    }
    return pattern;
}

void IR_Decision(IR_Array_T *ir_data)
{
    uint8_t pattern = IR_Array_GetPattern(ir_data);
	uint8_t stop_command =0;
	switch(pattern) {
	    case 0b0000: Motor_Forward();break;
	    case 0b0100: Motor_Left();break;
	    case 0b0010: Motor_Right();break;
	    case 0b0001: Motor_SharpRight();break;
	    case 0b1000: Motor_SharpLeft();break;
	    case 0b1100: Motor_SharpLeft();break;
	    case 0b0011: Motor_SharpRight();break;
	    case 0b1111: Motor_SharpLeft();stop_command++;break;
	}
}
