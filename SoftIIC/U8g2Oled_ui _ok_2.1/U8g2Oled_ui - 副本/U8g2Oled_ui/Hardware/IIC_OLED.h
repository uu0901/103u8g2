#ifndef __IIC_OLED_H__
#define __IIC_OLED_H__
#include "STM32f10x.h"

void OLED12864_IoInit(void);//IO初始化
void IIC_OLED_SDA_HIGH(void);	
void IIC_OLED_SDA_LOW(void);
void IIC_OLED_SCL_HIGH(void);	
void IIC_OLED_SCL_LOW(void);
#endif
