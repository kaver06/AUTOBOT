```
#include "main.h"
#include "ir_array.h"
#include <stdio.h>

UART_HandleTypeDef huart2;   // using USART2 for debugging

IR_Array_T ir_data;

// Redirect printf → UART
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    printf("IR Array Test Start\r\n");

    while (1)
    {
        IR_Array_Read(&ir_data);                   // read all 4 sensors
        uint8_t pattern = IR_Array_GetPattern(&ir_data);  // pack into single byte

        // Print sensor states
        printf("Sensors: L=%d ML=%d MR=%d R=%d | Pattern=0x%X\r\n",
               ir_data.state[0], ir_data.state[1],
               ir_data.state[2], ir_data.state[3],
               pattern);

        HAL_Delay(500); // 0.5 second delay for readability
    }
}

static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);
}
