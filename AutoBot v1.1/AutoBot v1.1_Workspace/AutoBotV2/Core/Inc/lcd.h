/*
 * lcd.h
 *
 *  Created on: Feb 21, 2026
 *      Author: kaver
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

#include "stm32f4xx_hal.h"

void lcd_init(I2C_HandleTypeDef *hi2c);
void lcd_clear(void);
void lcd_set_cursor(uint8_t row, uint8_t col);
void lcd_print(char *str);



#endif /* INC_LCD_H_ */
