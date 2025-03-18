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
1��׼��һ���������е�MDKģ��
2������u8g2��Դ���u8g2��stm32ʵ��ģ��
Դ�룺http://github.com/olikraus/u8g2
    STM32ʵ��ģ�壺http://github.com/nikola-v/u8g2_template_stm32f103c8t6
3��ȥ�����õ������ļ���ֻ����u8x8_d_ssd1306_128x64noname.c
4������u8g2_d_setup.c��ֻ����u8g2_Setup_ssd1306_i2c_128x64_noname_f()
5������u8g2_d_memory.c��ֻ����u8g2_m_16_8_f()
6���Թ��̽������ò��޳��������
7������u8x8_gpio_and_dalay_stm32()
8����u8g2���г�ʼ��
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
        // ��Ȼ�����ɶ��ˣ�
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
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*"C:\Users\ROG\Desktop\����\����.bmp",0*/
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
  0x00,/*"C:\Users\ROG\Desktop\����\�ղ�.bmp",0*/
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
  0x00, 0x00, 0x00, 0x00, /*"C:\Users\ROG\Desktop\����\Ͷ��.bmp",0*/
  /* (36 X 36 )*/

};
void PrintfVarFormat(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y,const uint8_t *font, short num)
{
    char buff[10];
    u8g2_SetFont(u8g2,font);//�趨����
    sprintf(buff,"%4d",num);//4�ǳ��ȵ����� 0�ǲ���4λǰ�油0 ��numΪ100 ��ӡ������0100 �˺����ķ���ֵ��buff�ĳ���
    u8g2_DrawStr(u8g2,x,y,buff);
}
    
void MyTest(void)
{
//    u8g2_ClearBuffer(&u8g2);                                              //��ջ�����
//    
//                                                                          //����
//    Delay_ms(1000);
//    u8g2_DrawLine(&u8g2, 0,0, 127, 63);         //���0 0�յ�127 63
//    u8g2_DrawLine(&u8g2, 127,0, 0, 63);         //���127 0�յ�0 63
//    u8g2_SendBuffer(&u8g2);                     //���ͻ�����
//    Delay_ms(1000);
//                                                                          //����

//    u8g2_ClearBuffer(&u8g2);                                             //��ջ����� 
    
//    draw(&u8g2);                                                          //�ٷ����Ժ���
    
//    u8g2_DrawXBMP(&u8g2, 0, 0, 25, 25, u8g_logo_bits);                    //����ͼ��
    
//    u8g2_DrawBox(&u8g2, 30, 0, 25, 25);                                    //����ʵ�ķ���
  
//    u8g2_SetFont(&u8g2,u8g2_font_helvR08_tr);                             //��������
//    u8g2_DrawButtonUTF8(&u8g2,72, 20, U8G2_BTN_BW2, 0, 2, 2, "Btn" );     //���Ʒ���+�ı�
    
//    u8g2_DrawCircle(&u8g2,10, 35, 10, U8G2_DRAW_ALL);                   //���ƿ���Բ

//    u8g2_DrawDisc(&u8g2,30, 35, 10, U8G2_DRAW_ALL);                     //����ʵ��Բ

//    u8g2_DrawEllipse(&u8g2,50, 45, 15, 10, U8G2_DRAW_ALL);              //���ƿ�����Բ

//    u8g2_DrawFilledEllipse(&u8g2,80, 45, 15, 10, U8G2_DRAW_ALL);        //����ʵ����Բ

//    u8g2_DrawFrame(&u8g2,0,50,25,13);                                   //���ƿ��ľ���
    
//    u8g2_SetFont(&u8g2,u8g2_font_unifont_t_symbols);                   //��������

//    u8g2_DrawGlyph(&u8g2,108, 20, 0x2603);                             //���ƹٷ�����
//    u8g2_DrawGlyphX2(&u8g2,96,60, 0x2603);                             //2�����ƹٷ�����
//    u8g2_DrawGlyphX2(&u8g2,96,60, 0x2614);                             //���ƹٷ�����

//    u8g2_DrawHLine(&u8g2, 0, 10, 50);                                  //����ˮƽ��

//    u8g2_DrawLine(&u8g2, 0,0, 127, 63);                                //����������

//    u8g2_DrawRBox(&u8g2,  0, 0, 50, 40, 15);                            //����Բ��ʵ�ķ���

//    u8g2_DrawRFrame(&u8g2,  0, 45, 50, 15, 5);                          //����Բ�ǿ��ķ���
    
//    u8g2_SetFont(&u8g2,u8g2_font_ncenB14_tr);                          //��������
//    u8g2_DrawStr(&u8g2,0,15,"Hello World!");                            //��ʾ�ַ���

//    u8g2_DrawTriangle(&u8g2, 0, 0, 50, 50,50, 0);                       //����������
    
//    u8g2_DrawPixel(&u8g2, 100, 30);                                     //����һ�����ص�

//    u8g2_SetFont(&u8g2,u8g2_font_unifont_t_symbols);                   //��������
//    u8g2_DrawUTF8(&u8g2,5, 20, "Snowman: �?);                      //��ʾ�ַ���+�ٷ�ͼ��

//    u8g2_DrawVLine(&u8g2, 50, 10, 30);                                  //���ƴ�ֱ��


//    u8g2_SetFont(&u8g2,u8g2_font_ncenB14_tr);                         //��������
//    u8g2_DrawStr(&u8g2,5,20,"Hello World!");                            //��ʾ�ַ�������������Ļ�ȡ��׼�����Ϻ����µĲ���
    
    //��ȡ��׼�����ϵĸ߶�
//    i = u8g2_GetAscent(&u8g2);//����i
//    PrintfVarFormat(&u8g2, 0,40,u8g2_font_unifont_t_symbols, i);//��ʾi
    //��ȡ��׼�����ϵĸ߶�
    
    //��ȡ��׼�����µĸ߶�
//    i = u8g2_GetDescent(&u8g2);//����i
//    PrintfVarFormat(&u8g2, 0,60,u8g2_font_unifont_t_symbols, i);//��ʾi
    //��ȡ��׼�����µĸ߶�
    
    //��ȡ��Ļ�߶�
//    i = u8g2_GetDisplayHeight(&u8g2);
//    PrintfVarFormat(&u8g2, 0,20,u8g2_font_ncenB14_tr, i);//��ʾi
    //��ȡ��Ļ���
//    i = u8g2_GetDisplayWidth(&u8g2);
//    PrintfVarFormat(&u8g2, 0,40,u8g2_font_ncenB14_tr, i);//��ʾi

    
//    u8g2_SendBuffer(&u8g2);                 //���ͻ�����
    
    
                                                    //���ƽ�����
//    char buff[20];
//    for( i=0;i<=100;i=i+1)
//    {
//        u8g2_ClearBuffer(&u8g2); 
//        u8g2_SetFont(&u8g2,u8g2_font_ncenB08_tf);//����
//        sprintf(buff,"%d%%",(int)(i/100.0*100));
//        u8g2_DrawStr(&u8g2,105,30,buff);        //��ǰ������ʾ
//        u8g2_DrawBox(&u8g2,2,22,i,10);          //����ʵ�ľ��ο�
//        u8g2_DrawFrame(&u8g2,0,20,103,14);      //���ľ��ο�
//        u8g2_SendBuffer(&u8g2);
//    }
                                                    //���ƽ�����
//        LED1_ON();
//        Delay_ms(500);
//        LED1_OFF();
//        Delay_ms(500);

//uint8_t keynum1;//ȫ�ֱ���
//uint8_t keynum2;//ȫ�ֱ���
//        u8g2_ClearBuffer(&u8g2);                                           //��ջ�����       
//        keynum1 = get_io_val(0);
//        keynum2 = get_io_val(1);
//        PrintfVarFormat(&u8g2, 0,20,u8g2_font_ncenB14_tr, keynum1);//��ʾi
//        PrintfVarFormat(&u8g2, 0,40,u8g2_font_ncenB14_tr, keynum2);//��ʾi
//        u8g2_SendBuffer(&u8g2);                                             //���ͻ�����
}



u8g2_t u8g2;


//****************************�˵�����****************************//
int8_t list_len;                            //�˵��б�������Ԫ�ص�����
uint8_t i;                                  //�����˵��б�������Ԫ�ص������ı���


int8_t ui_select = 0; //��ǰUI��ѡ��
int8_t y_select = 0; //��ǰ���õ��ı���ѡ��
int8_t ui_index = 0; //��ǰUI���ĸ�����
int8_t ui_state = 0;//UI״̬��

int8_t menu_x, menu_x_trg; //�˵�x �Ͳ˵���xĿ��
int8_t menu_y, menu_y_trg; //�˵�y �Ͳ˵���yĿ��


int8_t pic_x, pic_x_trg; //ͼƬҳ��x ��ͼƬ��xĿ��
int8_t pic_y, pic_y_trg; //ͼƬҳ��y ��ͼƬ��yĿ��

int8_t setting_x = 0, setting_x_trg = 0; //���ý���x �����ý���xĿ��
int8_t setting_y = 18, setting_y_trg = 18; //���ý���y �����ý���yĿ��

int8_t frame_len, frame_len_trg; //���Ŀ��
int8_t frame_y, frame_y_trg; //����y
//�˵��ṹ��
typedef struct
{
    char *str;
    uint8_t len;//�˵���Ӣ�Ŀ�����Strlen����ȡ���ȣ�Ϊ�˻�ȡ���ĵĳ�������Ҫ�����һ��len
}SETTING_LIST;

//�˵��б�
SETTING_LIST list[] = //�����б�
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
  E_MENU,   //�˵�����
  E_PIC,    //ͼƬ����
  E_SETTING,  //���ý���
  E_UI_MAX, //�������ֵ
} E_UI;

enum
{
  E_SELECT_SETTING, //�˵������ѡ��->����
  E_SELECT_PIC,     //�˵������ѡ��->ͼƬ
};

enum
{
  E_NONE,           //����״̬�� ��
  E_STATE_MENU,     //����״̬->�˵�����
  E_STATE_RUN_MENU, //����״̬->����˵�
  E_STATE_RUN_SETTING,//����״̬->��������
  E_STATE_RUN_PIC,//����״̬->����ͼƬ
  E_STATE_ARRIVE, //����״̬->����
};
//****************************�˵�����****************************//

//****************************��������****************************//
//������ֵ�ṹ��
typedef struct
{
  uint8_t val;         //��ǰ��ȡ�ĵ�ƽ
  uint8_t last_val;    //��һ�εĵ�ƽ
  uint8_t change;      //��Ǹı�
  uint8_t press_tick;  //����ʱ��
  uint8_t long_press_flag; //������־ ���������ɿ���ʱ���⵽
} KEY_T;
//����������������ʼ��
KEY_T key[2] = {0};

//��ֵ��Ϣ�ṹ��
typedef struct
{
  uint8_t id;        //������
  uint8_t press;       //�Ƿ���
  uint8_t update_flag; //�Ƿ�����
  uint8_t long_press;  //�Ƿ񳤰�
} KEY_MSG;
//�����ֵ��Ϣ����ʼ��
KEY_MSG key_msg = {0};
int key_long_press_tick = 0;//������������

//��ȡ������ƽֵ��ch=0������1,       ch=1������2
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
//����ɨ�躯��
void key_scan(void)
{
  for (int i = 0; i < 2; i++)
  {
    key[i].val =  get_io_val(i); //��ȡ��ǰ�İ���״̬
    if (key[i].val != key[i].last_val) //������������ı�
    {
      key[i].last_val = key[i].val; //����״̬
      key[i].change = 1;            //��¼�ı�
    }
  }
}

void key_proc(void)
{
  for (int i = 0; i < 2; i++)
  {
    if (key[i].change == 1) //��������ı��� ������Ϊ�а�������/�ɿ���
    {
      key[i].change = 0;//�����־
      if (key[i].val == 0)//�������
      {
        key_long_press_tick = 10; //��ʼ��������ʱ
      }
      else //����ɿ�
      {
        if (key[i].long_press_flag) //���������
        {
          key[i].long_press_flag = 0; //����ɿ���־
          key[i].press_tick = 0;//���̧�����
        }
        else
        {
          key[i].press_tick = 1;//���û���� ����Ϊ�Ƕ̰� ����
        }
      }
    }
  }
}
void key_down_cb(void)//�˺����ǰ����ɿ����м��� ��Ϊ�����̰�
{
  for (int i = 0; i < 2; i++)
  {
    if (key[i].press_tick) //����а�������
    {
      key[i].press_tick--;  //������һ
      if (!key[i].press_tick) //���Ϊ0
      {
        key_msg.id = i; //���Ͷ̰�msg
        key_msg.press = 1;
        key_msg.update_flag = 1;
        key_msg.long_press = 0;
      }
    }
  }
}
uint8_t ccc;
void key_press_cb(void) //�����ص� 1ms����һ��
{
//          ccc++;
//      printf("%d\r\n", ccc);
  if (key_long_press_tick != 0)//�������
  {

    key_long_press_tick--;//--
    if (key_long_press_tick == 0) //�����ʱʱ�����
    {
      for (int i = 0; i < 2; i++)
      {
        if (key[i].val == 0)//�������� ������а��µ� ��֤������Ϊ����
        {
          key_msg.id = i;
          key_msg.press = 1;
          key_msg.update_flag = 1;
          key_msg.long_press = 1;
          key[i].long_press_flag = 1; //���ͳ���msg
        }
      }
    }
  }
  key_down_cb();
}
void key_init(void) //������ʼ��
{
  for (int i = 0; i < 2; i++)
  {
    key[i].val = key[i].last_val = get_io_val(i);//��������ʼ�����ϵ�ʱ���״̬
  }
}
//****************************��������****************************//

//******************************************************ui����******************************************************//
//�˶��ƽ�����������Ŀ��ֵ����1�����򷵻�0
int8_t ui_run(int8_t *a, int8_t *a_trg, uint8_t step, uint8_t slow_cnt)
{
    uint8_t temp;
    temp = abs(*a_trg - *a) > slow_cnt ? step : 1;//Ŀ��ֵ�͵�ǰֵ��ѡ�񲽽�
    if(*a < *a_trg) *a += temp;
    else if(*a > *a_trg) *a -= temp;
    else    return 0;
    return 1;
}
void menu_ui_show(short offset_y) //�˵�ui��ʾ
{
  u8g2_DrawXBMP(&u8g2, menu_x - 40, offset_y + 20, 36, 35, dianzan);
  u8g2_DrawXBMP(&u8g2, menu_x + 45, offset_y + 20, 36, 36, toubi);
}

void setting_ui_show(int8_t offset_y) //����UI��ʾ
{ 
  int list_len = sizeof(list) / sizeof(SETTING_LIST);

  for (int i = 0 ; i < list_len; i++)
  {
    u8g2_DrawStr(&u8g2, setting_x + 2, offset_y + i * 20 + 18, list[i].str); // ��һ�����λ��
  }
  u8g2_DrawRFrame(&u8g2,setting_x, offset_y + frame_y, frame_len, 22, 3);

  ui_run(&frame_y, &frame_y_trg, 5, 4);
  ui_run(&frame_len, &frame_len_trg, 10, 5);
}

void pic_ui_show(short offset_y) //ͼƬUI��ʾ
{
  u8g2_DrawXBMP(&u8g2, 40, offset_y + 20, 36, 37, shoucang);
}

void menu_proc(KEY_MSG *msg) //�˵�������
{
  int list_len = 2 ;//�˵���Ԫ�ظ���

  if (msg->update_flag && msg->press) //����а�������
  {
    msg->update_flag = 0;//���������־
    if (msg->long_press == 0)//����Ƕ̰�
    {
      ui_state = E_STATE_MENU;//״̬������Ϊ�˵�����״̬
      if (msg->id)//�жϰ���id
      {
        ui_select++;//����
        if (ui_select >= list_len)
        {
          ui_select = list_len - 1;
        }
      }
      else
      {
        ui_select--;//����
        if (ui_select < 0)
        {
          ui_select = 0;
        }
      }
      menu_x_trg = ui_select * 90;//�޸Ĳ˵���xĿ��ֵ ʵ�ֲ˵�����
    }
    else
    {
      msg->long_press = 1;//�������
      if (msg->id == 1)//�жϰ���id
      {
        if (ui_select == E_SELECT_SETTING)//���Ŀ��uiΪ����
        {
          ui_index = E_SETTING;//�ѵ�ǰ��UI��Ϊ����ҳ�棬һ��������Ĵ�����������
          ui_state = E_STATE_RUN_SETTING; //UI״̬����Ϊ ��������
          menu_y_trg = -64;//���ò˵������Ŀ��
          setting_y_trg = 0;//�������ý����Ŀ��
          setting_y = 64;//�������ý���ĳ�ʼֵ
          ui_select = 0;
        }
        else if (ui_select == E_SELECT_PIC)//�����ͼƬ
        {
          ui_index = E_PIC;//ͬ��
          ui_state = E_STATE_RUN_PIC;//ͬ��
          menu_y_trg = -64;
          pic_y_trg = 0;
          pic_y = 64;
        }
      }
    }
  }
  switch (ui_state) //�˵�����UI״̬��
  {
    case E_STATE_MENU://�˵�״̬
      {
        ui_run(&menu_x, &menu_x_trg, 10, 4);//ֻ����x��
        menu_ui_show(0);//��ʾ
        break;
      }

    case E_STATE_RUN_SETTING://������������
      {
        static uint8_t flag = 0;//����һ��������������ж��Ƿ�λ
        if (ui_run(&setting_y, &setting_y_trg, 10, 4) == 0)
        {
          flag |= 0xf0;//������õ�λ�� ����0xF0
            printf("%x\r\n", flag);
        }
        if (ui_run(&menu_y, &menu_y_trg, 10, 4) == 0)
        {
          flag |= 0x0f;//����˵���λ�� ����0x0F
            printf("%x\r\n", flag);
        }
        if (flag == 0xff)// 0 | 0xf0 | 0x0f = 0xff
        { //�����λ�� ״̬��Ϊ��λ
          flag = 0;			 //�����־
          ui_state = E_STATE_ARRIVE;
        }
        menu_ui_show(menu_y);//��ʾ�˵�UI
        setting_ui_show(setting_y);//��ʾ����ui
        break;
      }
    case E_STATE_RUN_PIC://ͬ��
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
    default://Ĭ�� ��λ����none ��ʾ�˵�ui
      menu_ui_show(menu_y);
      break;
  }
}


void pic_proc(KEY_MSG *msg)//����UI��proc��������࣬Ҫע��ui_selectΪ���پͻ������ĸ�proc
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
  u8 index;//UI������
  void (*cb)(KEY_MSG*);//ui��ִ�к���
} UI_LIST;

UI_LIST ui_list[] = //UI��
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
    if (ui_index == ui_list[i].index)//�����ǰ��������UI���е�����
    {
      if (ui_list[i].cb)//ִ��UI��Ӧ�Ļص�����
      {
        u8g2_ClearBuffer(&u8g2);         // ����ڲ�������
        ui_list[i].cb(msg);
        u8g2_SendBuffer(&u8g2);          // transfer internal memory to the
      }
    }
  }
}
uint16_t c;
//******************************************************ui����******************************************************//
int main(void)
{
    uint8_t i;
    LED_Init();
    Key_Init();
    key_init();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);     //�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�

    uart_init(115200);
    printf("Hello\r\n");
    OLED_Init();                         //IIC OLED �˿ڳ�ʼ��
    
    //u8g2��ʼ��

    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R2, u8x8_byte_hw_i2c, u8x8_gpio_and_delay);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
    //u8g2��ʼ��
    u8g2_SetFont(&u8g2,u8g2_font_t0_22_mf);                         //��������
    
    frame_len = frame_len_trg = list[ui_select].len * 13;            //���Ŀ�����ã�Ĭ��ui_select��0��Ҳ����һ���˵���len�����ȣ�����ÿ���ַ��ĳ��ȣ�6�����ǿ�������Ŀ��
    ui_index = E_MENU;//����UIĬ�Ͻ���˵�
    TIM2_Int_Init(1000 - 1, 72 - 1);
//    TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //���TIMx�����жϱ�־ 

    while (1)
    {
        key_scan();
        key_press_cb();
        key_proc(); //����ɨ��
        ui_proc(&key_msg);
        
//        u8g2_ClearBuffer(&u8g2);                                           //��ջ�����       
//        
//        u8g2_DrawXBMP(&u8g2,40, 20, 36, 35, dianzan);

//        
//        u8g2_SendBuffer(&u8g2);   
//        printf("%d\r\n",c);


    }
}

void TIM2_IRQHandler(void)   //TIM2�ж�
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
    {
//        c ++;
//        u8g2_ClearBuffer(&u8g2);                                           //��ջ�����       
//        PrintfVarFormat(&u8g2, 80,40,u8g2_font_ncenB14_tr, c);//��ʾi
//        u8g2_SendBuffer(&u8g2);   
        c++;


        
    }
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //���TIMx�����жϱ�־ 

}

