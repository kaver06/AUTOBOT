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

volatile float battery_voltage = 0.0f;
volatile int battery_percentage = 0;

volatile uint32_t battery_tick_counter = 0;
static volatile bool battery_read_in_progress = false;

static volatile uint16_t battery_raw = 0;
static volatile bool battery_sample_ready = false;

void Start_Battery_ADC_Read(void)
{
    if (!battery_read_in_progress)
    {
        battery_read_in_progress = true;
        HAL_ADC_Start_IT(&hadc1);
    }
}

void BatteryMonitor_Init(void)
{
    HAL_TIM_Base_Start_IT(&htim7);

    Start_Battery_ADC_Read();
}

void BatteryMonitor_Get(float *v, uint8_t *p)
{
    __disable_irq();
    *v = battery_voltage;
    *p = battery_percentage;
    __enable_irq();
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	 if (hadc->Instance == ADC1)
	    {
			battery_raw = HAL_ADC_GetValue(hadc);
			battery_sample_ready = true;
			battery_read_in_progress = false;
	    }
}

void Battery_Process(void)
{
    if (!battery_sample_ready)
        return;

    battery_sample_ready = false;
    const float vref = 3.27f;
    const float R1 = 100.0f, R2 = 33.0f;
    const float CAL_V = 1.025f;

    float v_adc = ((float)battery_raw / 4095.0f) * vref;
    float new_voltage = (v_adc * ((R1 + R2) / R2)) * CAL_V;

    if (battery_voltage == 0.0f)
    {
        battery_voltage = new_voltage;
    }
    else
    {
        battery_voltage = (SMOOTHING_ALPHA * new_voltage) +
                          ((1.0f - SMOOTHING_ALPHA) * battery_voltage);
    }

    float p = ((battery_voltage - MIN_BAT_V) /
               (MAX_BAT_V - MIN_BAT_V)) * 100.0f;

    if (p > 100.0f) p = 100.0f;
    if (p < 0.0f)   p = 0.0f;

    battery_percentage = (int)(p + 0.5f);
}
