/*
 * battery_status.h
 *
 *  Created on: Oct 23, 2025
 *      Author: kaver
 */

#ifndef INC_BATTERY_STATUS_H_
#define INC_BATTERY_STATUS_H_
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_BAT_V 12.9f
#define MIN_BAT_V 7.9f
#define SMOOTHING_ALPHA 0.05f
void BatteryMonitor_Init(void);
float BatteryMonitor_GetVoltage(void);
void Start_Battery_ADC_Read(void);
void BatteryMonitor_Get(float *v, uint8_t *p);
void Battery_Process(void);

extern volatile uint32_t battery_tick_counter;


#endif /* INC_BATTERY_STATUS_H_ */
