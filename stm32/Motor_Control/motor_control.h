/*
 * motor_control.h
 *
 *  Created on: Sep 18, 2025
 *      Author: kaver
 */

#ifndef INC_MOTOR_CONTROL_H_
#define INC_MOTOR_CONTROL_H_

#include "stm32f4xx_hal.h"

// Speed definitions (adjust based on ARR)
#define MAX_SPEED   (__HAL_TIM_GET_AUTORELOAD(&htim1))     // 100%
#define TURN_SPEED  (MAX_SPEED * 0.6f)                     // 60%
#define BACKWARD_SPEED  (MAX_SPEED * 0.4)   // or just a fixed number like 400


// Functions
void Motor_Init(void);
void Motor_Forward();
void Motor_Backward();
void Motor_Left(void);
void Motor_Right(void);
void Motor_SharpRight(void);
void Motor_SharpLeft(void);
void Motor_Stop(void);


#endif /* INC_MOTOR_CONTROL_H_ */
