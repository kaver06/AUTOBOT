/*
 * battery_status.c
 *
 *  Created on: Oct 23, 2025
 *      Author: kaver
 */


#include "battery_status.h"
#include "main.h"
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim7;

// Define (not extern) here:
volatile float battery_voltage = 0.0f;
volatile int battery_percentage = 0;

volatile uint32_t battery_tick_counter = 0;
static volatile bool battery_read_in_progress = false;

void Start_Battery_ADC_Read(void)
{
    if (!battery_read_in_progress)
    {
        battery_read_in_progress = true;
        HAL_ADC_Start_IT(&hadc1);
    }
}

float BatteryMonitor_GetVoltage(void)
{
    float voltage;
    __disable_irq();
    voltage = battery_voltage;
    __enable_irq();
    return voltage;
}

void BatteryMonitor_Init(void)
{
    // Start timer interrupt
    HAL_TIM_Base_Start_IT(&htim7);

    // Kickstart first read immediately
    Start_Battery_ADC_Read();
}

// ADC conversion complete interrupt handler
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	 if (hadc->Instance == ADC1)
	    {
	        uint32_t raw = HAL_ADC_GetValue(hadc);
	        const float vref = 3.3f;
	        const float R1 = 100.0f, R2 = 33.0f;
	        const float CAL_V = 1.0120f;

	        float v_adc = ((float)raw / 4095.0f) * vref;
	        float new_voltage = (v_adc * ((R1 + R2) / R2))*CAL_V;

	        // Apply exponential smoothing
	        battery_voltage = (SMOOTHING_ALPHA * new_voltage) +
	                          ((1.0f - SMOOTHING_ALPHA) * battery_voltage);

	        battery_percentage = (int)((battery_voltage / MAX_BAT_V) * 100.0f);

	        battery_read_in_progress = false;
	    }
}
