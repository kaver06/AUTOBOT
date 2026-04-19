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

#define MAX_BAT_V 13.2f
#define SMOOTHING_ALPHA 1.0f  // 0.0–1.0, lower = smoother
void BatteryMonitor_Init(void);
float BatteryMonitor_GetVoltage(void);
void Start_Battery_ADC_Read(void);

extern volatile float battery_voltage;
extern volatile int battery_percentage;
extern volatile uint32_t battery_tick_counter;


#endif /* INC_BATTERY_STATUS_H_ */
