#include "uart.h"

volatile int esp32_tag_id = 0;
volatile float esp32_yaw = 0.0f;
volatile float esp32_pitch = 0.0f;
volatile float esp32_roll = 0.0f;
volatile float esp32_x = 0.0f;
volatile float esp32_y = 0.0f;
volatile float esp32_z = 0.0f;
volatile uint8_t new_esp32_data_flag = 0;
volatile uint8_t uart1_tx_busy = 0;
volatile uint32_t esp32_update_counter = 0;

uint8_t uart1_rx_buffer[RX_BUFFER_SIZE];
uint8_t uart1_data[RX_BUFFER_SIZE];
volatile uint8_t data_received_flag = 0;

void UART1_Init(void)
{
    HAL_UART_Receive_DMA(&huart1, uart1_rx_buffer, RX_BUFFER_SIZE);

    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

}

void UART1_SendString(const char *str)
{
    if (uart1_tx_busy)
        return;

    uart1_tx_busy = 1;

    if (HAL_UART_Transmit_IT(&huart1,(uint8_t *)str, strlen(str)) != HAL_OK)
    {
        uart1_tx_busy = 0;
    }
}

void UART1_ProcessReceivedData(void)
{
    if (data_received_flag)
    {
        data_received_flag = 0;

        // printf("ESP32 Data: %s\r\n", uart1_data);

        if (sscanf((char *)uart1_data, "%d,%f,%f,%f,%f,%f,%f",
                   &esp32_tag_id, &esp32_yaw, &esp32_pitch, &esp32_roll,
                   &esp32_x, &esp32_y, &esp32_z) == 7)
        {
            new_esp32_data_flag = 1;
            esp32_update_counter++;
            //printf("ESP RX: ID=%d X=%.2f Y=%.2f Z=%.2f\r\n",esp32_tag_id, esp32_x, esp32_y, esp32_z);
        }
    }
}
