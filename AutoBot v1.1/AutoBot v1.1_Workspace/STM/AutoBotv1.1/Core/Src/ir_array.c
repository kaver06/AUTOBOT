/*
 * ir_array.c
 *
 *  Created on: Nov 2, 2025
 *      Author: kaver
 */


#include "ir_array.h"
#include "uart.h"
#include "system_flags.h"
/*
 * left ir PC0
 * mid left PC1
 * mid right PA4
 * right ir PB0
 */

// Assign pins and ports for 4 sensors
const uint16_t ir_pins[IR_COUNT]  = {GPIO_PIN_0, GPIO_PIN_2, GPIO_PIN_4, GPIO_PIN_0};
GPIO_TypeDef* ir_ports[IR_COUNT]  = {GPIOB, GPIOG, GPIOA,GPIOC};
void IR_Array_Read(IR_Array_T *ir_data)
{
    for (int i = 0; i < IR_COUNT; i++)
    {
        ir_data->state[i] = HAL_GPIO_ReadPin(ir_ports[i], ir_pins[i]);
    }
}

void IR_PrintPatternBinary(uint8_t pattern)
{
    char bin[5];  // 4 bits + null terminator

    for (int i = 3; i >= 0; i--)
    {
        bin[3 - i] = ((pattern >> i) & 0x01) ? '1' : '0';
    }

    bin[4] = '\0';

    printf("IR Pattern: %s\r\n", bin);
}
uint8_t IR_Array_GetPattern(IR_Array_T *ir_data)
{
    uint8_t pattern = 0;
    for (int i = 0; i < IR_COUNT; i++)
    {
        pattern <<= 1;
        pattern |= (ir_data->state[i] & 0x01);
    }
   IR_PrintPatternBinary(pattern);
    return pattern;
}

void IR_PatternMonitor(uint8_t pattern)
{
    static uint8_t stop_cnt = 0;
    static uint8_t release_cnt = 0;
    static uint8_t esp_enabled = 0;

    if (pattern == 0b1111 || pattern == 0b1110 || pattern == 0b0111  )
    {
        stop_cnt++;
        release_cnt = 0;

        if (stop_cnt >= 3 && !esp_enabled)
        {
            esp_enabled = 1;
            osEventFlagsSet(obstacleFlags, ESP_APRILTAG_ENABLE);
        }
    }
    else
    {
        release_cnt++;
        stop_cnt = 0;

        if (release_cnt >= 3 && esp_enabled)
        {
            esp_enabled = 0;
            osEventFlagsClear(obstacleFlags, ESP_APRILTAG_ENABLE);
        }
    }
}

