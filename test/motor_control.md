```
#include "main.h"
#include "motor_control.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM1_Init();   // Timer configured from CubeMX

    Motor_Init();     // Start PWM

    // Example test sequence
    Motor_Forward(MAX_SPEED);  
    HAL_Delay(2000);

    Motor_Left();
    HAL_Delay(1000);

    Motor_Right();
    HAL_Delay(1000);

    Motor_Backward(TURN_SPEED);  
    HAL_Delay(2000);

    Motor_Stop();

    while (1) {
        // Future: Line following / Obstacle avoidance logic
    }
}
