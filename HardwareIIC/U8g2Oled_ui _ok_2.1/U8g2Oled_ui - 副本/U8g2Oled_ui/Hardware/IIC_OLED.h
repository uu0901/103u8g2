#ifndef __IIC_OLED_H__
#define __IIC_OLED_H__
#include "STM32f10x.h"

// void OLED12864_IoInit(void);//IO初始化
// void IIC_OLED_SDA_HIGH(void);	
// void IIC_OLED_SDA_LOW(void);
// void IIC_OLED_SCL_HIGH(void);	
// void IIC_OLED_SCL_LOW(void);

void OLED_ColorTurn(u8 i);
void OLED_DisplayTurn(u8 i);

void Oled_I2C_WaitEvent(I2C_TypeDef *I2Cx, uint32_t I2C_EVENT);
void I2C_WriteByte(uint8_t addr, uint8_t data);
void WriteCmd(unsigned char I2C_Command);
void WriteDat(unsigned char I2C_Data);
void OLED_DisPlay_On(void);
void OLED_DisPlay_Off(void);

void OLED_Init(void);
#endif
