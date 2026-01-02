#include "uart.h"

volatile int esp32_tag_id = 0;
volatile float esp32_yaw = 0.0f;
volatile float esp32_pitch = 0.0f;
volatile float esp32_roll = 0.0f;
volatile float esp32_x = 0.0f;
volatile float esp32_y = 0.0f;
volatile float esp32_z = 0.0f;
volatile uint8_t new_esp32_data_flag = 0;

// ====================== BUFFER DEFINITIONS ======================
uint8_t uart1_rx_buffer[RX_BUFFER_SIZE];     // DMA circular buffer
uint8_t uart1_data[RX_BUFFER_SIZE];          // Copy for processing
volatile uint8_t data_received_flag = 0;

// ====================== UART INITIALIZATION ======================
void UART1_Init(void)
{
    // The huart1 is already initialized by MX_USART1_UART_Init() in CubeMX
    // Just start DMA-based reception
    HAL_UART_Receive_DMA(&huart1, uart1_rx_buffer, RX_BUFFER_SIZE);

    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

}

// ====================== SEND STRING (NON-BLOCKING) ======================
void UART1_SendString(const char *str)
{
    HAL_UART_Transmit_IT(&huart1, (uint8_t *)str, strlen(str));
}

// ====================== PROCESS RECEIVED DATA ======================
void UART1_ProcessReceivedData(void)
{
    if (data_received_flag)
    {
        data_received_flag = 0;

        // Debug: optional print of raw data
        // printf("ESP32 Data: %s\r\n", uart1_data);

        // Parse incoming data
        if (sscanf((char *)uart1_data, "%d,%f,%f,%f,%f,%f,%f",
                   &esp32_tag_id, &esp32_yaw, &esp32_pitch, &esp32_roll,
                   &esp32_x, &esp32_y, &esp32_z) == 7)
        {
            new_esp32_data_flag = 1; // indicate new valid data received
        }
    }
}   // ✅ this closing brace was missing earlier!

// ====================== DMA RX COMPLETE CALLBACK ======================
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        memcpy(uart1_data, uart1_rx_buffer, RX_BUFFER_SIZE);
        data_received_flag = 1;

        // Restart DMA reception (important for continuous mode)
        HAL_UART_Receive_DMA(&huart1, uart1_rx_buffer, RX_BUFFER_SIZE);
    }
}

