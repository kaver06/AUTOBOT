#include "lcd.h"
#include <string.h>

#define LCD_ADDR (0x27 << 1)

static I2C_HandleTypeDef *lcd_i2c;

#define RS 0x01
#define EN 0x04
#define BL 0x08

static void lcd_write_nibble(uint8_t nibble)
{
    uint8_t data;

    data = nibble | BL;
    HAL_I2C_Master_Transmit(lcd_i2c, LCD_ADDR, &data, 1, 1000);

    data |= EN;
    HAL_I2C_Master_Transmit(lcd_i2c, LCD_ADDR, &data, 1, 1000);

    HAL_Delay(1);

    data &= ~EN;
    HAL_I2C_Master_Transmit(lcd_i2c, LCD_ADDR, &data, 1, 1000);
}

static void lcd_send(uint8_t value, uint8_t mode)
{
    uint8_t high = (value & 0xF0) | (mode ? RS : 0);
    uint8_t low  = ((value << 4) & 0xF0) | (mode ? RS : 0);

    lcd_write_nibble(high);
    lcd_write_nibble(low);
}

void lcd_init(I2C_HandleTypeDef *hi2c)
{
    lcd_i2c = hi2c;

    HAL_Delay(50);

    lcd_write_nibble(0x30);
    HAL_Delay(5);
    lcd_write_nibble(0x30);
    HAL_Delay(1);
    lcd_write_nibble(0x30);
    HAL_Delay(10);
    lcd_write_nibble(0x20);
    HAL_Delay(10);

    lcd_send(0x28, 0);
    lcd_send(0x08, 0);
    lcd_send(0x01, 0);
    HAL_Delay(2);
    lcd_send(0x06, 0);
    lcd_send(0x0C, 0);
}

void lcd_clear(void)
{
    lcd_send(0x01, 0);
    HAL_Delay(2);
}

void lcd_set_cursor(uint8_t row, uint8_t col)
{
    uint8_t addr = (row == 0) ? 0x80 + col : 0xC0 + col;
    lcd_send(addr, 0);
}

void lcd_print(char *str)
{
    while (*str)
    {
        lcd_send((uint8_t)(*str), 1);
        str++;
    }
}
