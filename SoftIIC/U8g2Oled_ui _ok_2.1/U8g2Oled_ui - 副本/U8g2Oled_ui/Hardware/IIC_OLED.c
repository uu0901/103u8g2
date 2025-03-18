#include "IIC_OLED.h"
/*
SDA=PA12;
SCL=PA15;	
*/
#define IIC_OLED_SDA_Pin        	GPIO_Pin_9
#define IIC_OLED_SDA_GPIO       	GPIOB
#define IIC_OLED_SDA_GPIO_CLK		RCC_APB2Periph_GPIOB

#define IIC_OLED_SCL_Pin        	GPIO_Pin_8
#define IIC_OLED_SCL_GPIO       	GPIOB
#define IIC_OLED_SCL_GPIO_CLK		RCC_APB2Periph_GPIOB

void OLED12864_IoInit(void)//IO初始化
{	
    GPIO_InitTypeDef GPIO_InitStructure = {0};						//定义GPIO结构体
	RCC_APB2PeriphClockCmd(IIC_OLED_SDA_GPIO_CLK,ENABLE);			//开启GPIO模块的时钟
	RCC_APB2PeriphClockCmd(IIC_OLED_SCL_GPIO_CLK,ENABLE);			//开启GPIO模块的时钟
		
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);			//关闭PA15的jtag调试功能
//	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);

	GPIO_InitStructure.GPIO_Pin=IIC_OLED_SDA_Pin;					//配置SDA端口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 				// 设置GPIO的模式为输出模式					// 设置端口输出类型为：推免输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(IIC_OLED_SDA_GPIO, &GPIO_InitStructure); 				// 初始化GPIO为高速推免输出模式

	GPIO_InitStructure.GPIO_Pin=IIC_OLED_SCL_Pin;					//配置SCL端口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 				// 设置GPIO的模式为输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 				// 配置 I/O 口的速度为：50MHz，还可选：GPIO_Speed_2MHz，GPIO_Speed_10MHz
	GPIO_Init(IIC_OLED_SCL_GPIO, &GPIO_InitStructure); 				// 初始化GPIO为高速推免输出模式
}
void IIC_OLED_SDA_HIGH(void)
{
	GPIO_SetBits(IIC_OLED_SDA_GPIO,IIC_OLED_SDA_Pin);
}
	
void IIC_OLED_SDA_LOW(void)
{
	GPIO_ResetBits(IIC_OLED_SDA_GPIO,IIC_OLED_SDA_Pin);
}
void IIC_OLED_SCL_HIGH(void)
{
	GPIO_SetBits(IIC_OLED_SCL_GPIO,IIC_OLED_SCL_Pin);
}
	
void IIC_OLED_SCL_LOW(void)
{
	GPIO_ResetBits(IIC_OLED_SCL_GPIO,IIC_OLED_SCL_Pin);
}

