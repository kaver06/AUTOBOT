/*
 * motor_control.c
 *
 *  Created on: Sep 18, 2025
 *      Author: kaver
 */
#include "motor_control.h"

// External handle for timer (declare the one from CubeMX)
extern TIM_HandleTypeDef htim1;

// Helper macros for readability
#define LPWM_A_CHANNEL TIM_CHANNEL_1
#define RPWM_A_CHANNEL TIM_CHANNEL_2
#define LPWM_B_CHANNEL TIM_CHANNEL_3
#define RPWM_B_CHANNEL TIM_CHANNEL_4

// -------- Initialization --------
void Motor_Init(void) {
    // Start PWM outputs for both motors
    HAL_TIM_PWM_Start(&htim1, LPWM_A_CHANNEL);
    HAL_TIM_PWM_Start(&htim1, RPWM_A_CHANNEL);
    HAL_TIM_PWM_Start(&htim1, LPWM_B_CHANNEL);
    HAL_TIM_PWM_Start(&htim1, RPWM_B_CHANNEL);

    Motor_Stop(); // ensure motors are off initially
}

// -------- Movement Functions --------

// Forward: both motors forward
void Motor_Forward() {
   // if (speed > __HAL_TIM_GET_AUTORELOAD(&htim1)) speed = __HAL_TIM_GET_AUTORELOAD(&htim1);

    __HAL_TIM_SET_COMPARE(&htim1, LPWM_A_CHANNEL, MAX_SPEED);
    __HAL_TIM_SET_COMPARE(&htim1, RPWM_A_CHANNEL, 0);

    __HAL_TIM_SET_COMPARE(&htim1, LPWM_B_CHANNEL, MAX_SPEED);
    __HAL_TIM_SET_COMPARE(&htim1, RPWM_B_CHANNEL, 0);
}

// Backward: both motors backward
void Motor_Backward() {
    //if (speed > __HAL_TIM_GET_AUTORELOAD(&htim1)) speed = __HAL_TIM_GET_AUTORELOAD(&htim1);

    __HAL_TIM_SET_COMPARE(&htim1, LPWM_A_CHANNEL, 0);
    __HAL_TIM_SET_COMPARE(&htim1, RPWM_A_CHANNEL, BACKWARD_SPEED);

    __HAL_TIM_SET_COMPARE(&htim1, LPWM_B_CHANNEL, 0);
    __HAL_TIM_SET_COMPARE(&htim1, RPWM_B_CHANNEL, BACKWARD_SPEED);
}

// Left: left motor slower, right motor full
void Motor_Left(void) {
    __HAL_TIM_SET_COMPARE(&htim1, LPWM_A_CHANNEL, TURN_SPEED); // Left motor 60%
    __HAL_TIM_SET_COMPARE(&htim1, RPWM_A_CHANNEL, 0);

    __HAL_TIM_SET_COMPARE(&htim1, LPWM_B_CHANNEL, MAX_SPEED);  // Right motor 100%
    __HAL_TIM_SET_COMPARE(&htim1, RPWM_B_CHANNEL, 0);
}

// Right: right motor slower, left motor full
void Motor_Right(void) {
    __HAL_TIM_SET_COMPARE(&htim1, LPWM_A_CHANNEL, MAX_SPEED);  // Left motor 100%
    __HAL_TIM_SET_COMPARE(&htim1, RPWM_A_CHANNEL, 0);

    __HAL_TIM_SET_COMPARE(&htim1, LPWM_B_CHANNEL, TURN_SPEED); // Right motor 60%
    __HAL_TIM_SET_COMPARE(&htim1, RPWM_B_CHANNEL, 0);
}

void Motor_SharpLeft(void) {
    __HAL_TIM_SET_COMPARE(&htim1, LPWM_A_CHANNEL, 0); // Left motor 60%
    __HAL_TIM_SET_COMPARE(&htim1, RPWM_A_CHANNEL, TURN_SPEED);

    __HAL_TIM_SET_COMPARE(&htim1, LPWM_B_CHANNEL, MAX_SPEED);  // Right motor 100%
    __HAL_TIM_SET_COMPARE(&htim1, RPWM_B_CHANNEL, 0);
}

void Motor_SharpRight(void) {
    __HAL_TIM_SET_COMPARE(&htim1, LPWM_A_CHANNEL, MAX_SPEED);  // Left motor 100%
    __HAL_TIM_SET_COMPARE(&htim1, RPWM_A_CHANNEL, 0);

    __HAL_TIM_SET_COMPARE(&htim1, LPWM_B_CHANNEL, 0); // Right motor 60%
    __HAL_TIM_SET_COMPARE(&htim1, RPWM_B_CHANNEL, MAX_SPEED);
}

// Stop: all channels off
void Motor_Stop(void) {
    __HAL_TIM_SET_COMPARE(&htim1, LPWM_A_CHANNEL, 0);
    __HAL_TIM_SET_COMPARE(&htim1, RPWM_A_CHANNEL, 0);

    __HAL_TIM_SET_COMPARE(&htim1, LPWM_B_CHANNEL, 0);
    __HAL_TIM_SET_COMPARE(&htim1, RPWM_B_CHANNEL, 0);
}

