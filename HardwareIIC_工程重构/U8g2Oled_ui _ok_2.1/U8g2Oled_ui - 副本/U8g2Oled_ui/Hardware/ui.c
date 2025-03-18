#include "ui.h"
#include "delay.h"
#include "ui_bmp_data.h"
#include "stdio.h"
#include "stdlib.h"
#include "led.h"
#include "iic_oled.h"
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

void PrintfVarFormat(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y,const uint8_t *font, short num)
{
    char buff[10];
    u8g2_SetFont(u8g2,font);//设定字体
    sprintf(buff,"%4d",num);//4是长度的限制 0是不足4位前面补0 如num为100 打印出来是0100 此函数的返回值是buff的长度
    u8g2_DrawStr(u8g2,x,y,buff);
}
void MyTest(void)
{
    u8g2_ClearBuffer(&u8g2);                                              //清空缓冲区
    
                                                                          //画线
    Delay_ms(1000);
    u8g2_DrawLine(&u8g2, 0,0, 127, 63);         //起点0 0终点127 63
    u8g2_DrawLine(&u8g2, 127,0, 0, 63);         //起点127 0终点0 63
    u8g2_SendBuffer(&u8g2);                     //发送缓冲区
    Delay_ms(1000);
                                                                          //画线

    u8g2_ClearBuffer(&u8g2);                                             //清空缓冲区 
    
    draw(&u8g2);                                                          //官方测试函数
    
    u8g2_DrawXBMP(&u8g2, 0, 0, 25, 25, u8g_logo_bits);                    //绘制图像
    
    u8g2_DrawBox(&u8g2, 30, 0, 25, 25);                                    //绘制实心方框
  
    u8g2_SetFont(&u8g2,u8g2_font_helvR08_tr);                             //设置字体
    u8g2_DrawButtonUTF8(&u8g2,72, 20, U8G2_BTN_BW2, 0, 2, 2, "Btn" );     //绘制方框+文本
    
    u8g2_DrawCircle(&u8g2,10, 35, 10, U8G2_DRAW_ALL);                   //绘制空心圆

    u8g2_DrawDisc(&u8g2,30, 35, 10, U8G2_DRAW_ALL);                     //绘制实心圆

    u8g2_DrawEllipse(&u8g2,50, 45, 15, 10, U8G2_DRAW_ALL);              //绘制空心椭圆

    u8g2_DrawFilledEllipse(&u8g2,80, 45, 15, 10, U8G2_DRAW_ALL);        //绘制实心椭圆

    u8g2_DrawFrame(&u8g2,0,50,25,13);                                   //绘制空心矩形
    
    u8g2_SetFont(&u8g2,u8g2_font_unifont_t_symbols);                   //设置字体

    u8g2_DrawGlyph(&u8g2,108, 20, 0x2603);                             //绘制官方符号
   u8g2_DrawGlyphX2(&u8g2,96,60, 0x2603);                             //2倍绘制官方符号
    u8g2_DrawGlyphX2(&u8g2,96,60, 0x2614);                             //绘制官方符号

    u8g2_DrawHLine(&u8g2, 0, 10, 50);                                  //绘制水平线

    u8g2_DrawLine(&u8g2, 0,0, 127, 63);                                //绘制两点线

    u8g2_DrawRBox(&u8g2,  0, 0, 50, 40, 15);                            //绘制圆角实心方框

    u8g2_DrawRFrame(&u8g2,  0, 45, 50, 15, 5);                          //绘制圆角空心方框
    
    u8g2_SetFont(&u8g2,u8g2_font_ncenB14_tr);                          //设置字体
    u8g2_DrawStr(&u8g2,0,15,"Hello World!");                            //显示字符串

    u8g2_DrawTriangle(&u8g2, 0, 0, 50, 50,50, 0);                       //绘制三角形
    
   u8g2_DrawPixel(&u8g2, 100, 30);                                     //绘制一个像素点

    u8g2_SetFont(&u8g2,u8g2_font_unifont_t_symbols);                   //设置字体
   u8g2_DrawUTF8(&u8g2, 5, 20, "Snowman: 鈽?");                  //显示字符串+官方图形

    u8g2_DrawVLine(&u8g2, 50, 10, 30);                                  //绘制垂直线


    u8g2_SetFont(&u8g2,u8g2_font_ncenB14_tr);                         //设置字体
    u8g2_DrawStr(&u8g2,5,20,"Hello World!");                            //显示字符串，方便下面的获取基准线以上和以下的测试
    uint8_t i = 0;
    //获取基准线以上的高度
    i = u8g2_GetAscent(&u8g2);//传递i
    PrintfVarFormat(&u8g2, 0,40,u8g2_font_unifont_t_symbols, i);//显示i
    //获取基准线以上的高度
    
    //获取基准线以下的高度
    i = u8g2_GetDescent(&u8g2);//传递i
    PrintfVarFormat(&u8g2, 0,60,u8g2_font_unifont_t_symbols, i);//显示i
    //获取基准线以下的高度
    
    //获取屏幕高度
    i = u8g2_GetDisplayHeight(&u8g2);
    PrintfVarFormat(&u8g2, 0,20,u8g2_font_ncenB14_tr, i);//显示i
    //获取屏幕宽度
    i = u8g2_GetDisplayWidth(&u8g2);
    PrintfVarFormat(&u8g2, 0,40,u8g2_font_ncenB14_tr, i);//显示i

    
    u8g2_SendBuffer(&u8g2);                 //发送缓冲区
    
    
                                                    //绘制进度条
    char buff[20];
    for( i=0;i<=100;i=i+1)
    {
        u8g2_ClearBuffer(&u8g2); 
        u8g2_SetFont(&u8g2,u8g2_font_ncenB08_tf);//字体
        sprintf(buff,"%d%%",(int)(i/100.0*100));
        u8g2_DrawStr(&u8g2,105,30,buff);        //当前进度显示
        u8g2_DrawBox(&u8g2,2,22,i,10);          //填充框实心矩形框
        u8g2_DrawFrame(&u8g2,0,20,103,14);      //空心矩形框
        u8g2_SendBuffer(&u8g2);
    }
                                                    //绘制进度条
        LED1_ON();
        Delay_ms(500);
        LED1_OFF();
        Delay_ms(500);

        uint8_t keynum1;//全局变量
        uint8_t keynum2;//全局变量
        u8g2_ClearBuffer(&u8g2);                                           //清空缓冲区       
        keynum1 = get_io_val(0);
       keynum2 = get_io_val(1);
        PrintfVarFormat(&u8g2, 0,20,u8g2_font_ncenB14_tr, keynum1);//显示i
      PrintfVarFormat(&u8g2, 0,40,u8g2_font_ncenB14_tr, keynum2);//显示i
       u8g2_SendBuffer(&u8g2);                                             //发送缓冲区
}

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
    return 3;
}
//****************************菜单部分****************************//
int8_t list_len;                             //菜单列表数组中元素的数量
uint8_t i;                                   //遍历菜单列表数组中元素的数量的变量


int8_t ui_select = 0;                        //当前UI的选项
int8_t y_select = 0;                         //当前设置的文本的选项
int8_t ui_index = 0;                         //当前UI在哪个界面
int8_t ui_state = 0;                         //UI状态机

int8_t menu_x, menu_x_trg;                  //菜单x 和菜单的x目标
int8_t menu_y, menu_y_trg;                  //菜单y 和菜单的y目标


int8_t pic_x, pic_x_trg;                    //图片页面x 和图片的x目标
int8_t pic_y, pic_y_trg;                    //图片页面y 和图片的y目标

int8_t setting_x = 0, setting_x_trg = 0;    //设置界面x 和设置界面x目标
int8_t setting_y = 18, setting_y_trg = 18;   //设置界面y 和设置界面y目标

int8_t frame_len, frame_len_trg;            //框框的宽度 和框框的宽度目标
int8_t frame_y, frame_y_trg;                //框框的y 和框框的y目标


int8_t Indic_y = 0, Indic_y_trg = 0;                           //指示器的y 和指示器y目标
int8_t Indic_x = 116, Indic_x_trg = 116;                           //指示器的y 和指示器y目标

int8_t Indic_vertical_x = 120, Indic_vertical_x_trg = 120;     //指示器垂线的x和指示器垂线的x目标
int8_t Indic_vertical_y = 0, Indic_vertical_y_trg = 0;     //指示器垂线的x和指示器垂线的x目标

int8_t Indic_Horizental_x = 118, Indic_Horizental_x_trg = 118;     //指示器垂线的x和指示器垂线的x目标
int8_t Indic_Horizental_y = 4, Indic_Horizental_y_trg = 4;     //指示器垂线的x和指示器垂线的x目标
//设置列表
SETTING_LIST list[] = 
{
    {"Led", 3},
    {"App", 3},
    {"Motor",5},
    {"Game", 4},
    {"Wether", 6},
    {"about", 5},
};

//****************************菜单部分****************************//

//****************************按键部分****************************//

//定义两个按键并初始化
KEY_T key[2] = {0};

 

//定义键值信息并初始化
KEY_MSG key_msg = {0};
int key_long_press_tick = 0;//按键长按计数


//按键扫描函数
void My_key_scan(void)
{
    for (int i = 0; i < 2; i++)
    {
        key[i].val =  get_io_val(i);        //获取当前的按键状态
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
        if (key[i].change == 1)                 //如果按键改变了 可以认为有按键按下/松开了
        { 
            key[i].change = 0;                  //清除标志
            if (key[i].val == 0)                //如果按下
            {
                key_long_press_tick = 10;       //开始长按倒计时
            }
            else                                //如果松开
            {
                if (key[i].long_press_flag)     //如果长按了
                {
                    key[i].long_press_flag = 0; //清除松开标志
                    key[i].press_tick = 0;      //清掉抬起计数
                }
                else
                {
                    key[i].press_tick = 1;      //如果没长按 则认为是短按计数
                }
            }
        }
    }
}
void key_down_cb(void)                      //此函数是按键松开后有计数，认为按键短按
{
    for (int i = 0; i < 2; i++)
    {
        if (key[i].press_tick)              //如果有按键计数
        {
            key[i].press_tick--;            //计数减一
            if (!key[i].press_tick)         //如果计数为0
            {
                key_msg.id = i;             //发送短按msg
                key_msg.press = 1;
                key_msg.update_flag = 1;
                key_msg.long_press = 0;    //非长按信息，也就是短按信息
            }
        }
    }
}
uint8_t ccc;
void key_press_cb(void)                 //按键回调 1ms调用一次
{
//          ccc++;
//      printf("%d\r\n", ccc);
    if (key_long_press_tick != 0)       //如果按下，长按倒计时不为0
    {
        key_long_press_tick--;          //长按倒计时自减
        if (key_long_press_tick == 0)  //如果长按倒计时自减到0也就是计时时间结束
        {
            for (int i = 0; i < 2; i++)//遍历按键 如果还有按下的 则证明按键为长按
            {
                if (key[i].val == 0)
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
void My_key_init(void) //按键初始化
{
    for (int i = 0; i < 2; i++)
    {
        key[i].val = key[i].last_val = get_io_val(i);//将按键初始化成上电时候的状态
    }
}
//****************************按键部分****************************//

//******************************************************ui部分******************************************************//
//运动逼近函数，到达目标值返回0，否则返回1
int8_t ui_run(int8_t *a, int8_t *a_trg, int8_t step, int8_t slow_cnt)
{
    uint8_t temp;
    temp = abs(*a_trg - *a) > slow_cnt ? step : 1;//目标值和当前值的选择步进
    if (*a < *a_trg) 
    {
        *a += temp;
    }
    else if (*a > *a_trg) 
    {
        *a -= temp;
    }
    else    
    {
        return 0;
    }
    return 1;
}
void menu_ui_show(short offset_y)       //菜单ui显示
{
    u8g2_DrawXBMP(&u8g2, menu_x - 40, offset_y + 20, 36, 35, dianzan);
    u8g2_DrawXBMP(&u8g2, menu_x + 45, offset_y + 20, 36, 36, toubi);
}
uint8_t backtoseeting;
void setting_ui_show(int8_t offset_y, int8_t offset_Indic_y, int8_t offset_Indic_vertical_x)   //设置UI显示
{
    int list_len = sizeof(list) / sizeof(SETTING_LIST);//list_len=6

    for (int i = 0 ; i < list_len; i++)//0到5 list[0].str~list[5].str
    {
        //drawstr的xy坐标是左下角，所以y轴要偏移，这里默认的y轴offset_y形参传入的是seeting_y也就是18，也就是这个字符串的高度是18，这里默认+18是因为第一个字符的i是0
        
        u8g2_DrawStr(&u8g2, setting_x + 6, offset_y + i * 20 + 18, list[i].str); // 第一段输出位置
    }
    
    u8g2_SetDrawColor(&u8g2,2);//异或显示，让框框反色，注意这里是要画RBox，Fram是不行的
    u8g2_DrawRBox(&u8g2, setting_x, offset_y + frame_y, frame_len, 22, 6);//初始化框框的y轴是offset_y是18，此时框框的y轴和第一个文本的y轴重复了
    u8g2_SetDrawColor(&u8g2,1);//异或显示，让框框反色，注意这里是要画RBox，Fram是不行的
   
    if(backtoseeting == 1)//回到设置标志位，目的是让框框回弹到list[0]
    {
        backtoseeting = 0;//清除该标志位
        ui_run(&frame_y, &frame_y_trg, 10, 10);       //框框y轴ui运动函数 
    }
    else
    {
        ui_run(&frame_y, &frame_y_trg, 5, 4);       //框框y轴ui运动函数 
    }
    ui_run(&frame_len, &frame_len_trg, 10, 5);  //框框宽度ui运动函数

    u8g2_DrawVLine(&u8g2, offset_Indic_vertical_x, Indic_vertical_y, 64);//垂线 左下角xy
    for(int i = 0; i < list_len; i++)//画list条水平线
    {
        //y 8~120
        //x 118                  y = 8                      len = （120-x)*2 = 4                 
        //box_x= x - 2=116       box_y = y+2 = 10          boxlen = (120 - box_x)* 2 = 8   box_ = h/2 = 4   
        u8g2_DrawHLine(&u8g2, Indic_Horizental_x, Indic_Horizental_y + i*8, 4);//左下角xy
    }
    u8g2_DrawRBox(&u8g2, Indic_x, offset_Indic_y + Indic_y , 8, 8, 4);//左上角xy   // offset_Indic_y = 4  Indic_y 
    ui_run(&Indic_y, &Indic_y_trg, 5, 4);                      //框框宽度ui运动函数   Indic_y_trg 没运动一次偏移一个指示器
    ui_run(&Indic_x, &Indic_x_trg, 5, 4);  
    ui_run(&Indic_Horizental_y, &Indic_Horizental_y_trg, 5, 4);                      //框框宽度ui运动函数   Indic_y_trg 没运动一次偏移一个指示器
    ui_run(&Indic_vertical_y, &Indic_vertical_y_trg, 5, 4);  

}

void pic_ui_show(short offset_y)        //图片UI显示
{

    u8g2_DrawXBMP(&u8g2, 40, offset_y + 20, 36, 37, shoucang);
}

void menu_proc(KEY_MSG *msg)            //菜单处理函数
{
    int list_len = 2 ;                  //菜单的元素个数

    if (msg->update_flag && msg->press) //如果有按键按下
    {
        msg->update_flag = 0;           //清除按键标志
        if (msg->long_press == 0)       //如果是短按
        {
            ui_state = E_STATE_MENU;    //状态机设置为菜单运行状态
            if (msg->id)                //id = 1 是key0
            {
                ui_select++;//增加
                if (ui_select >= list_len)//ui_select=2 --->ui_selec=1      限幅最大值为1
                {
                    ui_select = list_len - 1;
                }
            }
            else                         //id = 0 是keyup
            {
                ui_select--;//减少
                if (ui_select < 0)       //ui_select=-1 --->ui_selec=0      限幅最小值为0
                {
                    ui_select = 0;
                }
            }
            menu_x_trg = ui_select * 90;//修改菜单的x目标值 实现菜单滚动    menu_x_trg=1*90=90  or   menu_x_trg = 0
        }
        else                            //如果长按
        {
            msg->long_press = 1;
            if (msg->id == 1)           //id = 1 是key0
            {
                if (ui_select == E_SELECT_SETTING)//如果目标ui为设置 ui_select=E_SELECT_SETTING=0
                {
                    ui_index = E_SETTING;           //把当前的UI设为设置页面，一会会在他的处理函数中运算

                    ui_state = E_STATE_RUN_SETTING; //UI状态设置为 跑向设置

                    menu_y_trg = -64;               //设置菜单界面的目标

                    setting_y_trg = 0;              //设置设置界面的目标
                    setting_y = 64;                 //设置设置界面的初始值
                    
                    ui_select = 0;
                    Indic_y_trg  = 0;
                    Indic_vertical_y_trg = 0;
                    Indic_Horizental_y_trg = 4;
                }
                else if (ui_select == E_SELECT_PIC)//如果目标ui为设置 ui_select=E_SELECT_SETTING=1
                {
                    ui_index = E_PIC;               //把当前的UI设为图片页面，一会会在他的处理函数中运算

                    ui_state = E_STATE_RUN_PIC;     //UI状态设置为 跑向图片

                    menu_y_trg = -64;               //设置菜单界面的目标

                    pic_y_trg = 0;                  //设置图片界面的目标
                    pic_y = 64;                     //设置图片界面的初始值
                }
            }
        }
    }
    switch (ui_state)                                //菜单界面UI状态机
    {
        case E_STATE_MENU:                           //菜单运行，key0 keyup按键短按时，E_STATE_MENU=1
        {
            ui_run(&menu_x, &menu_x_trg, 10, 4);     //只运算x轴
            menu_ui_show(0);                         //显示
            break;
        }
        case E_STATE_RUN_SETTING:                   //运行跑向设置，key0按键长按时，E_STATE_RUN_SETTING=3
        {
            static uint8_t flag = 0;                //定义一个缓存变量用以判断是否到位
            if (ui_run(&setting_y, &setting_y_trg, 10, 4) == 0)//设置的y轴运动
            {
                flag |= 0xf0;                       //如果设置到位了 或上0xF0
                printf("%x\r\n", flag);
            }
            if (ui_run(&menu_y, &menu_y_trg, 10, 4) == 0)//菜单的y轴运动
            {
                flag |= 0x0f;                       //如果菜单到位了 或上0x0F
                printf("%x\r\n", flag);
            }
            if (flag == 0xff)                       // 0 | 0xf0 | 0x0f = 0xff
            {
                //如果到位了 状态置为到位
                flag = 0;                           //清除标志
                ui_state = E_STATE_ARRIVE;          //菜单状态为到位，E_STATE_ARRIVE=5
            }
            menu_ui_show(menu_y);                   //显示菜单UI
            setting_ui_show(setting_y,Indic_y, Indic_vertical_x);             //显示设置ui
            break;
        }
        case E_STATE_RUN_PIC:                       //运行跑向设置，key0按键长按时，E_STATE_RUN_PIC=4
        {
            static uint8_t flag = 0;                //定义一个缓存变量用以判断是否到位
            if (ui_run(&pic_y, &pic_y_trg, 10, 4) == 0)//图片的y轴运动
            {
                flag |= 0xf0;                      //如果图片到位了 或上0xF0     
            }
            if (ui_run(&menu_y, &menu_y_trg, 10, 4) == 0)//菜单的y轴运动
            {
                flag |= 0x0f;                      //如果菜单到位了 或上0x0F
            }
            if (flag == 0xff)                       // 0 | 0xf0 | 0x0f = 0xff
            {
                //如果到位了 状态置为到位
                flag = 0;                           //清除标志
                ui_state = E_STATE_ARRIVE;          //菜单状态为到位，E_STATE_ARRIVE=5
            }
            menu_ui_show(menu_y);                   //显示菜单UI
            pic_ui_show(pic_y);                     //显示图片ui
            break;
        }
        default:                                    //默认or到位ornone 显示菜单ui
            menu_ui_show(menu_y);
            break;
    }
}


void pic_proc(KEY_MSG *msg)                         //图片处理函数
{

    if (msg->update_flag && msg->press)             //如果有按键按下
    {
        msg->update_flag = 0;                       //清除按键标志
        if (msg->long_press == 1)                   //有按键长按了     
        {
            msg->long_press = 0;                    //清除长按标志位
            if (msg->id == 1)                       //id = 1是key0
            {

            }
            else                                    //id =0 是keyup
            {
                ui_index = E_MENU;                  //把当前的UI设为菜单页面，一会会在他的处理函数中运算    
                ui_state = E_STATE_RUN_PIC;         //UI状态设置为 跑向图片【其实是跑向菜单】

                menu_y_trg = 0;                     //设置菜单界面的目标   

                pic_y_trg = 64;                     //设置图片界面的目标
            }

        }
    }
    switch (ui_state)
    {
        case E_STATE_RUN_PIC:                               //运行跑向菜单，keyup按键长按时，E_STATE_RUN_PIC=4
        {
            static uint8_t pic_flag = 0;                    //定义一个缓存变量用以判断是否到位
            if (ui_run(&pic_y, &pic_y_trg, 10, 4) == 0)     //图片的y轴运动
            {
                pic_flag |= 0xf0;                           //如果图片到位了 或上0xF0     
            }
            if (ui_run(&menu_y, &menu_y_trg, 10, 4) == 0)   //菜单的y轴运动
            {
                pic_flag |= 0x0f;                            //如果菜单到位了 或上0x0F                        
            }
            if (pic_flag == 0xff)                            // 0 | 0xf0 | 0x0f = 0xff
            {
                 //如果到位了 状态置为到位
                pic_flag = 0;                               //清除标志
                ui_state = E_STATE_ARRIVE;                  //菜单状态为到位，E_STATE_ARRIVE=5
            }
            break;
        }
    }
    menu_ui_show(menu_y);            //显示菜单UI
    pic_ui_show(pic_y);              //显示菜单UI
}

/*
ui_select的情况
            ui_select
    list_1      0
    list_2      1
    list_3      2
    list_4      3 
    list_5      4
    list_6      5
*/

void setting_proc(KEY_MSG *msg)                              //设置处理函数
{
    
    int list_len = sizeof(list) / sizeof(SETTING_LIST);     //计算出列表的菜单个数

    if (msg->update_flag && msg->press)                     //按键更新了且按键按下了
    {
        msg->update_flag = 0;                               //清除按键更新
        if (msg->long_press == 0)                           //是短按
        {
            if (msg->id)                                    //id=1 是key0
            {
                ui_select ++;                                //选项+1
                if (ui_select >= list_len)                  //list_len=6  意味着框框要偏移6-1次，因为框框默认框选第一个菜单，因此限幅在list_len-1
                {
                    ui_select = list_len - 1;               //极限值为5
                }


                if (ui_select >= list_len / 2)             //光标大于3时，开始进行菜单偏移list_len/2=6/2=3
                {
                    y_select ++;                           //递增1 2 3
                }
                if (y_select >= list_len - 3)              //限幅菜单偏移3次
                {
                    y_select = list_len - 3;                
                }
            }
            else                                            //id=0 是key_up
            {
                ui_select --;                               //选项-1
                if (ui_select < 0)                          //list_len=6  意味着框框要偏移6-1次
                {
                    ui_select = 0;                          //极限值为0
                }


                if (ui_select < list_len / 2)               //光标小于3时，开始进行菜单偏移list_len/2=6/2=3             
                {
                    y_select --;                            //递减2 1 0
                }
                if (y_select <= 0)                          //限幅菜单偏移3次
                {
                    y_select = 0;
                }

            }
            setting_y_trg = -y_select * 20;                   //设置设置列表的文本偏移  光标向下和向上移动时，y_select都为正数，为了让菜单列表向上偏移，因此这里加了负号

            frame_y_trg = ui_select * 20;                    //设置框框的y轴偏移
            Indic_y_trg = ui_select * 4;                    //设置框框的y轴偏移
            frame_len_trg = list[ui_select].len * 13;        //设置框框的宽度偏移ui_select 0~5 对应list数组元素
        }
        else                                                //长按
        {
            msg->long_press = 0;
            if (msg->id == 1)                              //key0
            {
                if(ui_select == 0)                          //选中了LED
                {
                    ui_index = E_LED;                       //把当前的UI设为LED页面，一会会在他的处理函数中运算   

                    ui_state = E_STATE_RUN_LED;             //UI状态设置为 跑向LED

                    // menu_y_trg = 0;                     //设置LED界面的目标,暂未使用

                    setting_x_trg = -64;                   //设置设置界面的目标，向左偏移  
                    Indic_vertical_x_trg = -64;
                    Indic_Horizental_x_trg = -64;
                    Indic_x_trg = -64;
                    ui_select = 0;                         //光标默认设置为0    

                    
                }
            }
            else                                            //keyup
            {
                ui_index = E_MENU;                          //把当前的UI设为菜单页面，一会会在他的处理函数中运算   

                ui_state = E_STATE_RUN_SETTING;             //UI状态设置为 跑向设置【其实是跑向菜单】

                menu_y_trg = 0;                             //设置菜单界面的目标  

                setting_y_trg = 64;                         //设置设置界面的目标  
                Indic_y_trg  = 64;
                Indic_vertical_y_trg  = 64;
                Indic_Horizental_y_trg  = 64;

                       
                frame_y =  ui_select * 20 ;                  //框框的宽度设置，默认ui_select是0，也就是一个菜单的len（长度）乘以每个字符的长度（6）才是框框真正的宽度    
                ui_select = 0;                              //光标默认设置为0         

                y_select = 0;                               //页面偏移设置为默认
                frame_y_trg = 0;                            //框框y轴设置为0，显示函数那的函数会默认再次偏移18

                backtoseeting = 1;                          //回到设置标志位

                //框框宽度设置为第一个list,即list[0]
                frame_len_trg = list[ui_select].len * 13;        //设置框框的宽度偏移ui_select 0~5 对应list数组元素


            }

        }
    }

    switch (ui_state)
    {
        case E_STATE_RUN_SETTING:                                   //运行跑向菜单，keyup按键长按时，E_STATE_RUN_SETTING=3             
        {   
            static uint8_t flag = 0;                                //定义一个缓存变量用以判断是否到位
            
            if (ui_run(&setting_y, &setting_y_trg, 10, 4) == 0 )     //设置的y轴运动
            {
                flag |= 0xf0;                                      //如果图片到位了 或上0xF0                                       
            }
            if (ui_run(&Indic_Horizental_y ,&Indic_Horizental_y_trg, 10, 4) == 0 )     //设置的y轴运动
            {
                flag |= 0xf0;                                      //如果图片到位了 或上0xF0                                       
            }
            if (ui_run(&Indic_vertical_y, &Indic_vertical_y_trg, 10, 4) == 0 )     //设置的y轴运动
            {
                flag |= 0xf0;                                      //如果图片到位了 或上0xF0                                       
            }
            if (ui_run(&Indic_y, &Indic_y_trg, 10, 4) == 0 )     //设置的y轴运动
            {
                flag |= 0xf0;                                      //如果图片到位了 或上0xF0                                       
            }
            if (ui_run(&menu_y, &menu_y_trg, 10, 4) == 0)          //菜单的y轴运动
            {
                flag |= 0x0f;                                       //如果菜单到位了 或上0x0F       
            }
            if (flag == 0xff)                                       // 0 | 0xf0 | 0x0f = 0xff
            {
                 //如果到位了 状态置为到位
                flag = 0;                                           //清除标志
                ui_state = E_STATE_ARRIVE;                          //菜单状态为到位，E_STATE_ARRIVE=5
            }
            break;
        }
        case E_STATE_RUN_LED:                                   //运行跑向菜单，keyup按键长按时，E_STATE_RUN_SETTING=3             
        {   
            static uint8_t flag = 0;                                //定义一个缓存变量用以判断是否到位
            if (ui_run(&setting_x, &setting_x_trg, 10, 4) == 0)     //设置的y轴运动
            {
                flag |= 0xf0;                                      //如果图片到位了 或上0xF0                                       
            }
            if (ui_run(&Indic_vertical_x, &Indic_vertical_x_trg, 10, 4))       //菜单的y轴运动
            {
                flag |= 0x0f;                                       //如果菜单到位了 或上0x0F       
            }     
            if (ui_run(&Indic_Horizental_x, &Indic_Horizental_x_trg, 10, 4) == 0)     //设置的y轴运动
            {
                flag |= 0xf0;                                      //如果图片到位了 或上0xF0                                       
            }   
            if (ui_run(&Indic_x, &Indic_x_trg, 10, 4) == 0)     //设置的y轴运动
            {
                flag |= 0xf0;                                      //如果图片到位了 或上0xF0                                       
            }

            if (flag == 0xf0)                                       // 0 | 0xf0 | 0x0f = 0xff
            {
                 //如果到位了 状态置为到位
                flag = 0;                                           //清除标志
                ui_state = E_STATE_ARRIVE;                          //菜单状态为到位，E_STATE_ARRIVE=5
            }
            break;
        }
    }
    ui_run(&setting_y, &setting_y_trg, 5, 4);           //设置界面的ui动作函数
    menu_ui_show(menu_y);                               //显示菜单UI
    setting_ui_show(setting_y,Indic_y, Indic_vertical_x);                         //显示设置UI
}

void led_proc(KEY_MSG *msg)                         //LED处理函数
{
    static uint8_t ledflag = 2;
    if(ledflag == 0)    u8g2_DrawStr(&u8g2, 20, 20, "on");
    else                u8g2_DrawStr(&u8g2, 20, 40, "off");
    if (msg->update_flag && msg->press)             //如果有按键按下
    {
        msg->update_flag = 0;                       //清除按键标志
        if (msg->long_press == 1)                   //有按键长按了     
        {
            msg->long_press = 0;                    //清除长按标志位
            if (msg->id == 1)                       //id = 1是key0
            {

            }
            else                                    //id =0 是keyup
            {
                ui_index = E_SETTING;               //把当前的UI设为设置页面，一会会在他的处理函数中运算    
                ui_state = E_STATE_RUN_LED;         //UI状态设置为 跑向LED【其实是跑向设置】

                // menu_y_trg = 0;                  //设置LED界面的目标   
                Indic_vertical_x_trg = 120;
                Indic_Horizental_x_trg = 118;
                Indic_x_trg = 116;
                setting_x_trg = 0;                  //设置设置界面的目标
            }
        }
        else//是短按                        
        {
            msg->long_press = 0;
            if (msg->id == 1)                       //key0
            {
                LED1_ON();                            //亮
                ledflag = 0;
                

            }
            else
            {
                LED1_OFF();                            //灭
                ledflag = 1;
                
            }
        }
    }

    switch (ui_state)
    {
        case E_STATE_RUN_LED:                               //运行跑向菜单，keyup按键长按时，E_STATE_RUN_PIC=4
        {
            static uint8_t pic_flag = 0;                    //定义一个缓存变量用以判断是否到位

            if (ui_run(&setting_x, &setting_x_trg, 10, 4) == 0)     //图片的y轴运动
            {
                pic_flag |= 0xf0;                           //如果图片到位了 或上0xF0     
            }
            if (ui_run(&Indic_vertical_x, &Indic_vertical_x_trg, 10 , 4))   //菜单的y轴运动
            {
                pic_flag |= 0x0f;                            //如果菜单到位了 或上0x0F                        
            }
            if (ui_run(&Indic_Horizental_x, &Indic_Horizental_x_trg, 10, 4) == 0)     //设置的y轴运动
            {
                pic_flag |= 0xf0;                                      //如果图片到位了 或上0xF0                                       
            } 
            if (ui_run(&Indic_x, &Indic_x_trg, 10, 4) == 0)     //设置的y轴运动
            {
                pic_flag |= 0xf0;                                      //如果图片到位了 或上0xF0                                       
            }
            if (pic_flag == 0xf0)                            // 0 | 0xf0 | 0x0f = 0xff
            {
                 //如果到位了 状态置为到位
                pic_flag = 0;                               //清除标志
                ui_state = E_STATE_ARRIVE;                  //菜单状态为到位，E_STATE_ARRIVE=5
            }
            break;
        }
    }
    // menu_ui_show(menu_y);            //显示菜单UI
    setting_ui_show(setting_y, Indic_y, Indic_vertical_x);                         //显示设置UI
}

UI_LIST ui_list[] = //UI表
{
    {E_MENU,    menu_proc},
    {E_PIC,     pic_proc  },
    {E_SETTING, setting_proc},
    {E_LED,     led_proc},
};
void ui_proc(KEY_MSG *msg)
{
    int i;

    for (i = 0 ; i < E_UI_MAX ; i++)            //E_UI_MAX=3        E_MENU=0    E_PIC=1      E_SETTING=2
    {
        if (ui_index == ui_list[i].index)       //如果当前索引等于UI表中的索引
        {
            if (ui_list[i].cb)                  //执行UI对应的回调函数
            {
                u8g2_ClearBuffer(&u8g2);         // 清除内部缓冲区
                ui_list[i].cb(msg);              //执行对应的回调函数，并传入按键信息作为形参
                u8g2_SendBuffer(&u8g2);          // transfer internal memory to the
            }
        }
    }
}
