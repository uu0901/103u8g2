#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "LED.h"
#include "Key.h"
#include "u8g2.h"
#include "iic_oled.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "timer2.h"
#include "usart.h"	 
/*
1、准备一个正常运行的MDK模板
2、下载u8g2的源码和u8g2的stm32实例模板
源码：http://github.com/olikraus/u8g2
    STM32实例模板：http://github.com/nikola-v/u8g2_template_stm32f103c8t6
3、去掉无用的驱动文件，只保留u8x8_d_ssd1306_128x64noname.c
4、精简u8g2_d_setup.c，只保留u8g2_Setup_ssd1306_i2c_128x64_noname_f()
5、精简u8g2_d_memory.c，只保留u8g2_m_16_8_f()
6、对工程进行设置并剔除编译错误
7、更改u8x8_gpio_and_dalay_stm32()
8、对u8g2进行初始化
*/
//uint8_t u8g2_gpio_and_delay_stm32(U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr)
//{
//    switch(msg)
//        {
//            case U8X8_MSG_DELAY_MILLI://Function which implements a delay, arg_int contains the amount of ms
//                Delay_ms(arg_int);
//            break;
//            case U8X8_MSG_DELAY_10MICRO://Function which delays 10us
//                Delay_us(10);
//            break;  
//            case U8X8_MSG_DELAY_100NANO://Function which delays 100ns
//                __NOP();
//            break;
//            case U8X8_MSG_GPIO_I2C_CLOCK:
//                if (arg_int) IIC_OLED_SCL_HIGH();
//                else IIC_OLED_SCL_LOW();
//            break;
//            case U8X8_MSG_GPIO_I2C_DATA:
//                if (arg_int) IIC_OLED_SDA_HIGH();
//            else IIC_OLED_SDA_LOW();
//            break;
//            default:
//                return 0; //A message was received which is not implemented, return 0 to indicate an error
//    }
//    return 1; // command processed successfully.
//}
#define SCL_Pin GPIO_Pin_10
#define SDA_Pin GPIO_Pin_11
#define IIC_GPIO_Port GPIOB
uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch (msg)
    {
    case U8X8_MSG_DELAY_100NANO: // delay arg_int * 100 nano seconds
        __NOP();
        break;
    case U8X8_MSG_DELAY_10MICRO: // delay arg_int * 10 micro seconds
        for (uint16_t n = 0; n < 320; n++)
        {
            __NOP();
        }
        break;
    case U8X8_MSG_DELAY_MILLI: // delay arg_int * 1 milli second
        Delay_ms(1);
        break;
    case U8X8_MSG_DELAY_I2C: // arg_int is the I2C speed in 100KHz, e.g. 4 = 400 KHz
        Delay_us(5);
        break;                    // arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
    case U8X8_MSG_GPIO_I2C_CLOCK: // arg_int=0: Output low at I2C clock pin
        if (arg_int == 1)
        {
            GPIO_SetBits(IIC_GPIO_Port, SCL_Pin);
        }
        else if (arg_int == 0)
        {
            GPIO_ResetBits(IIC_GPIO_Port, SCL_Pin);
        }
        break;                   // arg_int=1: Input dir with pullup high for I2C clock pin
    case U8X8_MSG_GPIO_I2C_DATA: // arg_int=0: Output low at I2C data pin
        if (arg_int == 1)
        {
            GPIO_SetBits(IIC_GPIO_Port, SDA_Pin);
        }
        else if (arg_int == 0)
        {
            GPIO_ResetBits(IIC_GPIO_Port, SDA_Pin);
        }
        break; // arg_int=1: Input dir with pullup high for I2C data pin
    case U8X8_MSG_GPIO_MENU_SELECT:
        u8x8_SetGPIOResult(u8x8, /* get menu select pin state */ 0);
        break;
    case U8X8_MSG_GPIO_MENU_NEXT:
        u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
        break;
    case U8X8_MSG_GPIO_MENU_PREV:
        u8x8_SetGPIOResult(u8x8, /* get menu prev pin state */ 0);
        break;
    case U8X8_MSG_GPIO_MENU_HOME:
        u8x8_SetGPIOResult(u8x8, /* get menu home pin state */ 0);
        break;
    default:
        u8x8_SetGPIOResult(u8x8, 1); // default return value
        break;
    }
    return 1;
}
uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    static uint8_t buffer[32]; /* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
    static uint8_t buf_idx;
    uint8_t *data;

    switch (msg)
    {

    case U8X8_MSG_BYTE_SEND:
        data = (uint8_t *)arg_ptr;
        while (arg_int > 0)
        {
            buffer[buf_idx++] = *data;
            data++;
            arg_int--;
        }
        break;

    case U8X8_MSG_BYTE_INIT:
        /* add your custom code to init i2c subsystem */
        break;

    case U8X8_MSG_BYTE_START_TRANSFER:
        buf_idx = 0;
        break;

    case U8X8_MSG_BYTE_END_TRANSFER:

        //		HW_I2cWrite(u8x8,buffer,buf_idx);
        // 居然给我蒙对了！
        if (buf_idx <= 0)
            break;

        /* wait for the busy falg to be reset */
        // while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

        /* start transfer */
        I2C_GenerateSTART(I2C2, ENABLE);
        Oled_I2C_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT);

        I2C_Send7bitAddress(I2C2, u8x8_GetI2CAddress(u8x8), I2C_Direction_Transmitter);
        Oled_I2C_WaitEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);

        for (uint8_t i = 0; i < buf_idx; i++)
        {
            I2C_SendData(I2C2, buffer[i]);

            Oled_I2C_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED);
        }

        I2C_GenerateSTOP(I2C2, ENABLE);

        break;

    default:
        return 0;
    }
    return 1;
}
void draw(u8g2_t *u8g2)
{
    u8g2_ClearBuffer(u8g2); 
    u8g2_SetFontMode(u8g2, 1); 
    u8g2_SetFontDirection(u8g2, 0); 
    u8g2_SetFont(u8g2, u8g2_font_inb24_mf);
    u8g2_DrawStr(u8g2, 0, 20, "U");
    
    u8g2_SetFontDirection(u8g2, 1);
    u8g2_SetFont(u8g2, u8g2_font_inb30_mn);
    u8g2_DrawStr(u8g2, 21,8,"8");
        
    u8g2_SetFontDirection(u8g2, 0);
    u8g2_SetFont(u8g2, u8g2_font_inb24_mf);
    u8g2_DrawStr(u8g2, 51,30,"g");
    u8g2_DrawStr(u8g2, 67,30,"\xb2");
    
    u8g2_DrawHLine(u8g2, 2, 35, 47);
    u8g2_DrawHLine(u8g2, 3, 36, 47);
    u8g2_DrawVLine(u8g2, 45, 32, 12);
    u8g2_DrawVLine(u8g2, 46, 33, 12);
  
    u8g2_SetFont(u8g2, u8g2_font_4x6_tr);
    u8g2_DrawStr(u8g2, 1,54,"github.com/olikraus/u8g2");
    
    u8g2_SendBuffer(u8g2);
    Delay_ms(1000);
}
static const unsigned char u8g_logo_bits[] U8X8_PROGMEM =
{
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x00,0x00,0x00,0xE0,0x01,0x00,0x00,0xF0,0x01,0x00,0x00,0xF8,0x01,0x00,0x00,0xFC,0x01,0x00,0x00,0xFE,0x01,0x00,
    0x30,0xFE,0xFF,0x01,0x3C,0xFE,0xFF,0x01,0x3C,0xFE,0xFF,0x01,0x3C,0xFE,0xFF,0x01,0x3C,0xFE,0xFF,0x01,0x3C,0xFE,0xFF,0x01,0x3C,0xFE,0xFF,0x01,0x3C,0xFE,0xFF,0x01,
    0x3C,0xFE,0xFF,0x00,0x3C,0xFE,0xFF,0x00,0x3C,0xFE,0xFF,0x00,0x3C,0xFE,0x7F,0x00,0x3C,0xFE,0x7F,0x00,0x3C,0xFE,0x7F,0x00,0x3C,0xFE,0x3F,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,

};
static const unsigned  char dianzan[] U8X8_PROGMEM = {
  0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x3E,
  0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00,
  0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x00,
  0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x00,
  0x7F, 0x00, 0x00, 0x00, 0x80, 0x7F, 0x00, 0x00,
  0x00, 0x80, 0x7F, 0x00, 0x00, 0x00, 0xC0, 0x7F,
  0x00, 0x00, 0x00, 0xE0, 0x7F, 0x00, 0x00, 0x00,
  0xF8, 0x7F, 0x00, 0x00, 0xF0, 0xF8, 0xFF, 0xFF,
  0x01, 0xFC, 0xF8, 0xFF, 0xFF, 0x07, 0xFC, 0xF8,
  0xFF, 0xFF, 0x07, 0xFE, 0xF8, 0xFF, 0xFF, 0x07,
  0xFE, 0xF8, 0xFF, 0xFF, 0x07, 0xFE, 0xF8, 0xFF,
  0xFF, 0x07, 0xFE, 0xF8, 0xFF, 0xFF, 0x07, 0xFE,
  0xF8, 0xFF, 0xFF, 0x07, 0xFE, 0xF8, 0xFF, 0xFF,
  0x03, 0xFE, 0xF8, 0xFF, 0xFF, 0x03, 0xFE, 0xF8,
  0xFF, 0xFF, 0x03, 0xFE, 0xF8, 0xFF, 0xFF, 0x03,
  0xFE, 0xF8, 0xFF, 0xFF, 0x01, 0xFE, 0xF8, 0xFF,
  0xFF, 0x01, 0xFE, 0xF8, 0xFF, 0xFF, 0x01, 0xFE,
  0xF8, 0xFF, 0xFF, 0x01, 0xFE, 0xF8, 0xFF, 0xFF,
  0x00, 0xFE, 0xF8, 0xFF, 0xFF, 0x00, 0xFC, 0xF8,
  0xFF, 0x7F, 0x00, 0xFC, 0xF8, 0xFF, 0x3F, 0x00,
  0xF8, 0xF8, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*"C:\Users\ROG\Desktop\三连\点赞.bmp",0*/
  /* (36 X 35 )*/
};

static const unsigned  char shoucang[] U8X8_PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00,
  0x00, 0x1F, 0x00, 0x00, 0x00, 0x80, 0x1F, 0x00,
  0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0xC0,
  0x3F, 0x00, 0x00, 0x00, 0xC0, 0x3F, 0x00, 0x00,
  0x00, 0xC0, 0x7F, 0x00, 0x00, 0x00, 0xE0, 0xFF,
  0x00, 0x00, 0x00, 0xF0, 0xFF, 0x01, 0x00, 0x00,
  0xFC, 0xFF, 0x03, 0x00, 0xE0, 0xFF, 0xFF, 0xFF,
  0x00, 0xFC, 0xFF, 0xFF, 0xFF, 0x07, 0xFE, 0xFF,
  0xFF, 0xFF, 0x07, 0xFC, 0xFF, 0xFF, 0xFF, 0x07,
  0xFC, 0xFF, 0xFF, 0xFF, 0x03, 0xF8, 0xFF, 0xFF,
  0xFF, 0x01, 0xF0, 0xFF, 0xFF, 0xFF, 0x00, 0xE0,
  0xFF, 0xFF, 0x7F, 0x00, 0xC0, 0xFF, 0xFF, 0x3F,
  0x00, 0x80, 0xFF, 0xFF, 0x3F, 0x00, 0x00, 0xFF,
  0xFF, 0x1F, 0x00, 0x00, 0xFF, 0xFF, 0x1F, 0x00,
  0x00, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0xFF, 0xFF,
  0x0F, 0x00, 0x00, 0xFF, 0xFF, 0x0F, 0x00, 0x00,
  0xFF, 0xFF, 0x0F, 0x00, 0x00, 0xFF, 0xFF, 0x0F,
  0x00, 0x00, 0xFF, 0xFF, 0x1F, 0x00, 0x00, 0xFF,
  0xFF, 0x1F, 0x00, 0x80, 0xFF, 0xF0, 0x1F, 0x00,
  0x80, 0x3F, 0xC0, 0x1F, 0x00, 0x80, 0x1F, 0x00,
  0x1F, 0x00, 0x00, 0x07, 0x00, 0x1C, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00,/*"C:\Users\ROG\Desktop\三连\收藏.bmp",0*/
  /* (36 X 37 )*/

};
static const unsigned char  toubi[] U8X8_PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x1F,
  0x00, 0x00, 0x00, 0xF0, 0xFF, 0x01, 0x00, 0x00,
  0xFC, 0xFF, 0x07, 0x00, 0x00, 0xFF, 0xFF, 0x0F,
  0x00, 0x80, 0xFF, 0xFF, 0x1F, 0x00, 0xC0, 0xFF,
  0xFF, 0x7F, 0x00, 0xE0, 0x07, 0x00, 0x7C, 0x00,
  0xF0, 0x03, 0x00, 0xFC, 0x00, 0xF0, 0x03, 0x00,
  0xFC, 0x01, 0xF8, 0xFF, 0xF1, 0xFF, 0x01, 0xF8,
  0xFF, 0xF1, 0xFF, 0x03, 0xF8, 0x7F, 0xC0, 0xFF,
  0x03, 0xFC, 0x1F, 0x00, 0xFF, 0x03, 0xFC, 0x07,
  0x00, 0xFE, 0x07, 0xFC, 0x07, 0x01, 0xFC, 0x07,
  0xFC, 0xC3, 0x31, 0xF8, 0x07, 0xFC, 0xE1, 0xF1,
  0xF8, 0x07, 0xFC, 0xF1, 0xF1, 0xF0, 0x07, 0xFC,
  0xF1, 0xF1, 0xF0, 0x07, 0xFC, 0xF1, 0xF1, 0xF1,
  0x07, 0xFC, 0xF1, 0xF1, 0xF1, 0x07, 0xFC, 0xF1,
  0xF1, 0xF1, 0x03, 0xF8, 0xF1, 0xF1, 0xF1, 0x03,
  0xF8, 0xFF, 0xF1, 0xFF, 0x03, 0xF8, 0xFF, 0xF1,
  0xFF, 0x01, 0xF0, 0xFF, 0xF1, 0xFF, 0x01, 0xF0,
  0xFF, 0xF1, 0xFF, 0x00, 0xE0, 0xFF, 0xF1, 0x7F,
  0x00, 0xC0, 0xFF, 0xFF, 0x7F, 0x00, 0x80, 0xFF,
  0xFF, 0x3F, 0x00, 0x00, 0xFF, 0xFF, 0x0F, 0x00,
  0x00, 0xFC, 0xFF, 0x07, 0x00, 0x00, 0xF0, 0xFF,
  0x01, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, /*"C:\Users\ROG\Desktop\三连\投币.bmp",0*/
  /* (36 X 36 )*/

};
void PrintfVarFormat(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y,const uint8_t *font, short num)
{
    char buff[10];
    u8g2_SetFont(u8g2,font);//设定字体
    sprintf(buff,"%4d",num);//4是长度的限制 0是不足4位前面补0 如num为100 打印出来是0100 此函数的返回值是buff的长度
    u8g2_DrawStr(u8g2,x,y,buff);
}
    
void MyTest(void)
{
//    u8g2_ClearBuffer(&u8g2);                                              //清空缓冲区
//    
//                                                                          //画线
//    Delay_ms(1000);
//    u8g2_DrawLine(&u8g2, 0,0, 127, 63);         //起点0 0终点127 63
//    u8g2_DrawLine(&u8g2, 127,0, 0, 63);         //起点127 0终点0 63
//    u8g2_SendBuffer(&u8g2);                     //发送缓冲区
//    Delay_ms(1000);
//                                                                          //画线

//    u8g2_ClearBuffer(&u8g2);                                             //清空缓冲区 
    
//    draw(&u8g2);                                                          //官方测试函数
    
//    u8g2_DrawXBMP(&u8g2, 0, 0, 25, 25, u8g_logo_bits);                    //绘制图像
    
//    u8g2_DrawBox(&u8g2, 30, 0, 25, 25);                                    //绘制实心方框
  
//    u8g2_SetFont(&u8g2,u8g2_font_helvR08_tr);                             //设置字体
//    u8g2_DrawButtonUTF8(&u8g2,72, 20, U8G2_BTN_BW2, 0, 2, 2, "Btn" );     //绘制方框+文本
    
//    u8g2_DrawCircle(&u8g2,10, 35, 10, U8G2_DRAW_ALL);                   //绘制空心圆

//    u8g2_DrawDisc(&u8g2,30, 35, 10, U8G2_DRAW_ALL);                     //绘制实心圆

//    u8g2_DrawEllipse(&u8g2,50, 45, 15, 10, U8G2_DRAW_ALL);              //绘制空心椭圆

//    u8g2_DrawFilledEllipse(&u8g2,80, 45, 15, 10, U8G2_DRAW_ALL);        //绘制实心椭圆

//    u8g2_DrawFrame(&u8g2,0,50,25,13);                                   //绘制空心矩形
    
//    u8g2_SetFont(&u8g2,u8g2_font_unifont_t_symbols);                   //设置字体

//    u8g2_DrawGlyph(&u8g2,108, 20, 0x2603);                             //绘制官方符号
//    u8g2_DrawGlyphX2(&u8g2,96,60, 0x2603);                             //2倍绘制官方符号
//    u8g2_DrawGlyphX2(&u8g2,96,60, 0x2614);                             //绘制官方符号

//    u8g2_DrawHLine(&u8g2, 0, 10, 50);                                  //绘制水平线

//    u8g2_DrawLine(&u8g2, 0,0, 127, 63);                                //绘制两点线

//    u8g2_DrawRBox(&u8g2,  0, 0, 50, 40, 15);                            //绘制圆角实心方框

//    u8g2_DrawRFrame(&u8g2,  0, 45, 50, 15, 5);                          //绘制圆角空心方框
    
//    u8g2_SetFont(&u8g2,u8g2_font_ncenB14_tr);                          //设置字体
//    u8g2_DrawStr(&u8g2,0,15,"Hello World!");                            //显示字符串

//    u8g2_DrawTriangle(&u8g2, 0, 0, 50, 50,50, 0);                       //绘制三角形
    
//    u8g2_DrawPixel(&u8g2, 100, 30);                                     //绘制一个像素点

//    u8g2_SetFont(&u8g2,u8g2_font_unifont_t_symbols);                   //设置字体
//    u8g2_DrawUTF8(&u8g2,5, 20, "Snowman: 鈽?);                      //显示字符串+官方图形

//    u8g2_DrawVLine(&u8g2, 50, 10, 30);                                  //绘制垂直线


//    u8g2_SetFont(&u8g2,u8g2_font_ncenB14_tr);                         //设置字体
//    u8g2_DrawStr(&u8g2,5,20,"Hello World!");                            //显示字符串，方便下面的获取基准线以上和以下的测试
    
    //获取基准线以上的高度
//    i = u8g2_GetAscent(&u8g2);//传递i
//    PrintfVarFormat(&u8g2, 0,40,u8g2_font_unifont_t_symbols, i);//显示i
    //获取基准线以上的高度
    
    //获取基准线以下的高度
//    i = u8g2_GetDescent(&u8g2);//传递i
//    PrintfVarFormat(&u8g2, 0,60,u8g2_font_unifont_t_symbols, i);//显示i
    //获取基准线以下的高度
    
    //获取屏幕高度
//    i = u8g2_GetDisplayHeight(&u8g2);
//    PrintfVarFormat(&u8g2, 0,20,u8g2_font_ncenB14_tr, i);//显示i
    //获取屏幕宽度
//    i = u8g2_GetDisplayWidth(&u8g2);
//    PrintfVarFormat(&u8g2, 0,40,u8g2_font_ncenB14_tr, i);//显示i

    
//    u8g2_SendBuffer(&u8g2);                 //发送缓冲区
    
    
                                                    //绘制进度条
//    char buff[20];
//    for( i=0;i<=100;i=i+1)
//    {
//        u8g2_ClearBuffer(&u8g2); 
//        u8g2_SetFont(&u8g2,u8g2_font_ncenB08_tf);//字体
//        sprintf(buff,"%d%%",(int)(i/100.0*100));
//        u8g2_DrawStr(&u8g2,105,30,buff);        //当前进度显示
//        u8g2_DrawBox(&u8g2,2,22,i,10);          //填充框实心矩形框
//        u8g2_DrawFrame(&u8g2,0,20,103,14);      //空心矩形框
//        u8g2_SendBuffer(&u8g2);
//    }
                                                    //绘制进度条
//        LED1_ON();
//        Delay_ms(500);
//        LED1_OFF();
//        Delay_ms(500);

//uint8_t keynum1;//全局变量
//uint8_t keynum2;//全局变量
//        u8g2_ClearBuffer(&u8g2);                                           //清空缓冲区       
//        keynum1 = get_io_val(0);
//        keynum2 = get_io_val(1);
//        PrintfVarFormat(&u8g2, 0,20,u8g2_font_ncenB14_tr, keynum1);//显示i
//        PrintfVarFormat(&u8g2, 0,40,u8g2_font_ncenB14_tr, keynum2);//显示i
//        u8g2_SendBuffer(&u8g2);                                             //发送缓冲区
}



u8g2_t u8g2;


//****************************菜单部分****************************//
int8_t list_len;                            //菜单列表数组中元素的数量
uint8_t i;                                  //遍历菜单列表数组中元素的数量的变量


int8_t ui_select = 0; //当前UI的选项
int8_t y_select = 0; //当前设置的文本的选项
int8_t ui_index = 0; //当前UI在哪个界面
int8_t ui_state = 0;//UI状态机

int8_t menu_x, menu_x_trg; //菜单x 和菜单的x目标
int8_t menu_y, menu_y_trg; //菜单y 和菜单的y目标


int8_t pic_x, pic_x_trg; //图片页面x 和图片的x目标
int8_t pic_y, pic_y_trg; //图片页面y 和图片的y目标

int8_t setting_x = 0, setting_x_trg = 0; //设置界面x 和设置界面x目标
int8_t setting_y = 18, setting_y_trg = 18; //设置界面y 和设置界面y目标

int8_t frame_len, frame_len_trg; //框框的宽度
int8_t frame_y, frame_y_trg; //框框的y
//菜单结构体
typedef struct
{
    char *str;
    uint8_t len;//菜单是英文可以用Strlen来获取长度，为了获取中文的长度这里要再添加一个len
}SETTING_LIST;

//菜单列表
SETTING_LIST list[] = //设置列表
{
    {"LDQQ", 4},
    {"ABC", 3},
    {"YYDS-OLED", 9},
    {"ABCD", 4},
    {"ABCDF", 5},
    {"ABCDFG", 6},
};
enum
{
  E_MENU,   //菜单界面
  E_PIC,    //图片界面
  E_SETTING,  //设置界面
  E_UI_MAX, //界面最大值
} E_UI;

enum
{
  E_SELECT_SETTING, //菜单界面的选择->设置
  E_SELECT_PIC,     //菜单界面的选择->图片
};

enum
{
  E_NONE,           //运行状态机 无
  E_STATE_MENU,     //运行状态->菜单运行
  E_STATE_RUN_MENU, //运行状态->跑向菜单
  E_STATE_RUN_SETTING,//运行状态->跑向设置
  E_STATE_RUN_PIC,//运行状态->跑向图片
  E_STATE_ARRIVE, //运行状态->到达
};
//****************************菜单部分****************************//

//****************************按键部分****************************//
//按键键值结构体
typedef struct
{
  uint8_t val;         //当前读取的电平
  uint8_t last_val;    //上一次的电平
  uint8_t change;      //标记改变
  uint8_t press_tick;  //按下时间
  uint8_t long_press_flag; //长按标志 用来屏蔽松开的时候检测到
} KEY_T;
//定义两个按键并初始化
KEY_T key[2] = {0};

//键值信息结构体
typedef struct
{
  uint8_t id;        //按键号
  uint8_t press;       //是否按下
  uint8_t update_flag; //是否最新
  uint8_t long_press;  //是否长按
} KEY_MSG;
//定义键值信息并初始化
KEY_MSG key_msg = {0};
int key_long_press_tick = 0;//按键长按计数

//获取按键电平值，ch=0，按键1,       ch=1，按键2
uint8_t get_io_val(uint8_t ch)
{
    if(ch == 0) 
    {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == 0)
        {
            Delay_ms(20);
             if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == 0)
            {
                 return 0;
            }
        }
        else if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == 1)
        {
            Delay_ms(20);
             if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == 1)
            {
                 return 1;
            }
        }
    }
    else
    {
         if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) == 0)
        {
            Delay_ms(20);
             if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) == 0)
            {
                 return 0;
            }
        }
        else if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) == 1)
        {
            Delay_ms(20);
             if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) == 1)
            {
                 return 1;
            }
        }
        
    }
}
//按键扫描函数
void key_scan(void)
{
  for (int i = 0; i < 2; i++)
  {
    key[i].val =  get_io_val(i); //获取当前的按键状态
    if (key[i].val != key[i].last_val) //如果按键发生改变
    {
      key[i].last_val = key[i].val; //更新状态
      key[i].change = 1;            //记录改变
    }
  }
}

void key_proc(void)
{
  for (int i = 0; i < 2; i++)
  {
    if (key[i].change == 1) //如果按键改变了 可以认为有按键按下/松开了
    {
      key[i].change = 0;//清除标志
      if (key[i].val == 0)//如果按下
      {
        key_long_press_tick = 10; //开始长按倒计时
      }
      else //如果松开
      {
        if (key[i].long_press_flag) //如果长按了
        {
          key[i].long_press_flag = 0; //清除松开标志
          key[i].press_tick = 0;//清掉抬起计数
        }
        else
        {
          key[i].press_tick = 1;//如果没长按 则认为是短按 计数
        }
      }
    }
  }
}
void key_down_cb(void)//此函数是按键松开后有计数 认为按键短按
{
  for (int i = 0; i < 2; i++)
  {
    if (key[i].press_tick) //如果有按键计数
    {
      key[i].press_tick--;  //计数减一
      if (!key[i].press_tick) //如果为0
      {
        key_msg.id = i; //发送短按msg
        key_msg.press = 1;
        key_msg.update_flag = 1;
        key_msg.long_press = 0;
      }
    }
  }
}
uint8_t ccc;
void key_press_cb(void) //按键回调 1ms调用一次
{
//          ccc++;
//      printf("%d\r\n", ccc);
  if (key_long_press_tick != 0)//如果按下
  {

    key_long_press_tick--;//--
    if (key_long_press_tick == 0) //如果计时时间结束
    {
      for (int i = 0; i < 2; i++)
      {
        if (key[i].val == 0)//遍历按键 如果还有按下的 则证明按键为长按
        {
          key_msg.id = i;
          key_msg.press = 1;
          key_msg.update_flag = 1;
          key_msg.long_press = 1;
          key[i].long_press_flag = 1; //发送长按msg
        }
      }
    }
  }
  key_down_cb();
}
void key_init(void) //按键初始化
{
  for (int i = 0; i < 2; i++)
  {
    key[i].val = key[i].last_val = get_io_val(i);//将按键初始化成上电时候的状态
  }
}
//****************************按键部分****************************//

//******************************************************ui部分******************************************************//
//运动逼近函数，到达目标值返回1，否则返回0
int8_t ui_run(int8_t *a, int8_t *a_trg, uint8_t step, uint8_t slow_cnt)
{
    uint8_t temp;
    temp = abs(*a_trg - *a) > slow_cnt ? step : 1;//目标值和当前值的选择步进
    if(*a < *a_trg) *a += temp;
    else if(*a > *a_trg) *a -= temp;
    else    return 0;
    return 1;
}
void menu_ui_show(short offset_y) //菜单ui显示
{
  u8g2_DrawXBMP(&u8g2, menu_x - 40, offset_y + 20, 36, 35, dianzan);
  u8g2_DrawXBMP(&u8g2, menu_x + 45, offset_y + 20, 36, 36, toubi);
}

void setting_ui_show(int8_t offset_y) //设置UI显示
{ 
  int list_len = sizeof(list) / sizeof(SETTING_LIST);

  for (int i = 0 ; i < list_len; i++)
  {
    u8g2_DrawStr(&u8g2, setting_x + 2, offset_y + i * 20 + 18, list[i].str); // 第一段输出位置
  }
  u8g2_DrawRFrame(&u8g2,setting_x, offset_y + frame_y, frame_len, 22, 3);

  ui_run(&frame_y, &frame_y_trg, 5, 4);
  ui_run(&frame_len, &frame_len_trg, 10, 5);
}

void pic_ui_show(short offset_y) //图片UI显示
{
  u8g2_DrawXBMP(&u8g2, 40, offset_y + 20, 36, 37, shoucang);
}

void menu_proc(KEY_MSG *msg) //菜单处理函数
{
  int list_len = 2 ;//菜单的元素个数

  if (msg->update_flag && msg->press) //如果有按键按下
  {
    msg->update_flag = 0;//清楚按键标志
    if (msg->long_press == 0)//如果是短按
    {
      ui_state = E_STATE_MENU;//状态机设置为菜单运行状态
      if (msg->id)//判断按键id
      {
        ui_select++;//增加
        if (ui_select >= list_len)
        {
          ui_select = list_len - 1;
        }
      }
      else
      {
        ui_select--;//减少
        if (ui_select < 0)
        {
          ui_select = 0;
        }
      }
      menu_x_trg = ui_select * 90;//修改菜单的x目标值 实现菜单滚动
    }
    else
    {
      msg->long_press = 1;//如果长按
      if (msg->id == 1)//判断按键id
      {
        if (ui_select == E_SELECT_SETTING)//如果目标ui为设置
        {
          ui_index = E_SETTING;//把当前的UI设为设置页面，一会会在他的处理函数中运算
          ui_state = E_STATE_RUN_SETTING; //UI状态设置为 跑向设置
          menu_y_trg = -64;//设置菜单界面的目标
          setting_y_trg = 0;//设置设置界面的目标
          setting_y = 64;//设置设置界面的初始值
          ui_select = 0;
        }
        else if (ui_select == E_SELECT_PIC)//如果是图片
        {
          ui_index = E_PIC;//同上
          ui_state = E_STATE_RUN_PIC;//同上
          menu_y_trg = -64;
          pic_y_trg = 0;
          pic_y = 64;
        }
      }
    }
  }
  switch (ui_state) //菜单界面UI状态机
  {
    case E_STATE_MENU://菜单状态
      {
        ui_run(&menu_x, &menu_x_trg, 10, 4);//只运算x轴
        menu_ui_show(0);//显示
        break;
      }

    case E_STATE_RUN_SETTING://运行跑向设置
      {
        static uint8_t flag = 0;//定义一个缓存变量用以判断是否到位
        if (ui_run(&setting_y, &setting_y_trg, 10, 4) == 0)
        {
          flag |= 0xf0;//如果设置到位了 或上0xF0
            printf("%x\r\n", flag);
        }
        if (ui_run(&menu_y, &menu_y_trg, 10, 4) == 0)
        {
          flag |= 0x0f;//如果菜单到位了 或上0x0F
            printf("%x\r\n", flag);
        }
        if (flag == 0xff)// 0 | 0xf0 | 0x0f = 0xff
        { //如果到位了 状态置为到位
          flag = 0;			 //清除标志
          ui_state = E_STATE_ARRIVE;
        }
        menu_ui_show(menu_y);//显示菜单UI
        setting_ui_show(setting_y);//显示设置ui
        break;
      }
    case E_STATE_RUN_PIC://同上
      {
        static uint8_t flag = 0;
        if (ui_run(&pic_y, &pic_y_trg, 10, 4) == 0)
        {
          flag |= 0xf0;
        }
        if (ui_run(&menu_y, &menu_y_trg, 10, 4) == 0)
        {
          flag |= 0x0f;
        }
        if (flag == 0xff)
        {
          flag = 0;
          ui_state = E_STATE_ARRIVE;
        }
        menu_ui_show(menu_y);
        pic_ui_show(pic_y);
        break;
      }
    default://默认 到位或者none 显示菜单ui
      menu_ui_show(menu_y);
      break;
  }
}


void pic_proc(KEY_MSG *msg)//几个UI的proc函数都差不多，要注意ui_select为多少就会运行哪个proc
{

  if (msg->update_flag && msg->press)
  {
    msg->update_flag = 0;
    if (msg->long_press == 1)
    {
      msg->long_press = 0;
      if (msg->id == 1)
      {

      }
      else
      {
        ui_index = E_MENU;
        ui_state = E_STATE_RUN_PIC;
        menu_y_trg = -0;
        pic_y_trg = 64;
      }

    }
  }
  switch (ui_state)
  {
    case E_STATE_RUN_PIC:
      {
        static u8 pic_flag = 0;
        if (ui_run(&pic_y, &pic_y_trg, 10, 4) == 0)
        {
          pic_flag |= 0xf0;
        }
        if (ui_run(&menu_y, &menu_y_trg, 10, 4) == 0)
        {
          pic_flag |= 0x0f;
        }
        if (pic_flag == 0xff)
        {
          pic_flag = 0;
          ui_state = E_STATE_ARRIVE;
        }
        break;
      }
  }
  menu_ui_show(menu_y);
  pic_ui_show(pic_y);
}


void setting_proc(KEY_MSG *msg)
{
  int list_len = sizeof(list) / sizeof(SETTING_LIST);

  if (msg->update_flag && msg->press)
  {
    msg->update_flag = 0;
    if (msg->long_press == 0)
    {
      if (msg->id)
      {
        ui_select++;
        if (ui_select >= list_len)
        {
          ui_select = list_len - 1;
        }
        
        if(ui_select >= list_len / 2)
        {
            y_select ++;
        }
        if (y_select >= list_len - 3)
        {
          y_select = list_len - 3;
        }
      }
      else
      {
        ui_select--;
        if (ui_select < 0)
        {
          ui_select = 0;
        }
        if (ui_select < list_len /2)
        {
          y_select --;
        }
        if(y_select <= 0)
        {
            y_select = 0;
        }
        
      }
      setting_y_trg = -y_select * 20;
      frame_y_trg = ui_select * 20;
      frame_len_trg = list[ui_select].len * 13;
    }
    else
    {
      msg->long_press = 0;
      if (msg->id == 1)
      {

      }
      else
      {
        ui_index = E_MENU;
        ui_state = E_STATE_RUN_SETTING;
        menu_y_trg = -0;
        setting_y_trg = 64;
        ui_select = 0;
      }

    }
  }

  switch (ui_state)
  {
    case E_STATE_RUN_SETTING:
      {
        static u8 flag = 0;
        if (ui_run(&setting_y, &setting_y_trg, 10, 4) == 0)
        {
          flag |= 0xf0;
        }
        if (ui_run(&menu_y, &menu_y_trg, 10, 4) == 0)
        {
          flag |= 0x0f;
        }
        if (flag == 0xff)
        {
          flag = 0;
          ui_state = E_STATE_ARRIVE;
        }
        break;
      }

  }
    ui_run(&setting_y, &setting_y_trg, 5, 4);
  menu_ui_show(menu_y);
  setting_ui_show(setting_y);
}

typedef struct
{
  u8 index;//UI的索引
  void (*cb)(KEY_MSG*);//ui的执行函数
} UI_LIST;

UI_LIST ui_list[] = //UI表
{
  {E_MENU,    menu_proc},
  {E_PIC,     pic_proc  },
  {E_SETTING, setting_proc},
};

void ui_proc(KEY_MSG *msg)
{
  int i;

  for (i = 0 ; i < E_UI_MAX ; i++)
  {
    if (ui_index == ui_list[i].index)//如果当前索引等于UI表中的索引
    {
      if (ui_list[i].cb)//执行UI对应的回调函数
      {
        u8g2_ClearBuffer(&u8g2);         // 清除内部缓冲区
        ui_list[i].cb(msg);
        u8g2_SendBuffer(&u8g2);          // transfer internal memory to the
      }
    }
  }
}
uint16_t c;
//******************************************************ui部分******************************************************//
int main(void)
{
    uint8_t i;
    LED_Init();
    Key_Init();
    key_init();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);     //设置中断优先级分组为组2：2位抢占优先级，2位响应优先级

    uart_init(115200);
    printf("Hello\r\n");
    OLED_Init();                         //IIC OLED 端口初始化
    
    //u8g2初始化

    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R2, u8x8_byte_hw_i2c, u8x8_gpio_and_delay);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
    //u8g2初始化
    u8g2_SetFont(&u8g2,u8g2_font_t0_22_mf);                         //设置字体
    
    frame_len = frame_len_trg = list[ui_select].len * 13;            //框框的宽度设置，默认ui_select是0，也就是一个菜单的len（长度）乘以每个字符的长度（6）才是框框真正的宽度
    ui_index = E_MENU;//设置UI默认进入菜单
    TIM2_Int_Init(1000 - 1, 72 - 1);
//    TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //清除TIMx更新中断标志 

    while (1)
    {
        key_scan();
        key_press_cb();
        key_proc(); //按键扫描
        ui_proc(&key_msg);
        
//        u8g2_ClearBuffer(&u8g2);                                           //清空缓冲区       
//        
//        u8g2_DrawXBMP(&u8g2,40, 20, 36, 35, dianzan);

//        
//        u8g2_SendBuffer(&u8g2);   
//        printf("%d\r\n",c);


    }
}

void TIM2_IRQHandler(void)   //TIM2中断
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
    {
//        c ++;
//        u8g2_ClearBuffer(&u8g2);                                           //清空缓冲区       
//        PrintfVarFormat(&u8g2, 80,40,u8g2_font_ncenB14_tr, c);//显示i
//        u8g2_SendBuffer(&u8g2);   
        c++;


        
    }
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //清除TIMx更新中断标志 

}

