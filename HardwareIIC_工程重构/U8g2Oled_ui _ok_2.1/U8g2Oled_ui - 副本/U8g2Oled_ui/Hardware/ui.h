#ifndef __UI_H
#define __UI_H
#include "stm32f10x.h" 
#include "u8g2.h"
extern u8g2_t u8g2;

//键值信息结构体
typedef struct
{
    uint8_t id;                  //按键号
    uint8_t press;               //是否按下
    uint8_t update_flag;         //是否最新
    uint8_t long_press;          //是否长按
} KEY_MSG;
extern KEY_MSG key_msg;
//按键键值结构体
typedef struct
{
    uint8_t val;                 //当前读取的电平
    uint8_t last_val;            //上一次的电平
    uint8_t change;              //标记改变
    uint8_t press_tick;          //按下时间
    uint8_t long_press_flag;     //长按标志 用来屏蔽松开的时候检测到
} KEY_T;

//设置列表结构体
typedef struct
{
    char *str;
    uint8_t len;                            //菜单是英文可以用Strlen来获取长度，为了获取中文的长度这里要再添加一个len
} SETTING_LIST;
extern SETTING_LIST list[];

extern enum
{
    E_MENU,                  //菜单界面
    E_PIC,                   //图片界面
    E_SETTING,               //设置界面
    E_LED,                   //LED界面
    E_UI_MAX,                //界面最大值
} E_UI;
enum
{
    E_SELECT_SETTING,        //菜单界面的选择->进入设置
    E_SELECT_PIC,            //菜单界面的选择->进入图片
};

enum
{
    E_NONE,                 //运行状态机 无
    E_STATE_MENU,           //运行状态->菜单运行
    E_STATE_RUN_MENU,       //运行状态->跑向菜单
    E_STATE_RUN_SETTING,    //运行状态->跑向设置
    E_STATE_RUN_PIC,        //运行状态->跑向图片
    E_STATE_RUN_LED,        //运行状态->跑向图片
    E_STATE_ARRIVE,         //运行状态->到达
};
typedef struct
{
    uint8_t index;              //UI的索引
    void (*cb)(KEY_MSG*);       //ui的执行函数
} UI_LIST;
uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
void draw(u8g2_t *u8g2);
void PrintfVarFormat(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, const uint8_t *font, short num);
void MyTest(void);
uint8_t get_io_val(uint8_t ch);
void My_key_init(void);
void key_proc(void);
void ui_proc(KEY_MSG *msg);
void My_key_scan(void);
void key_proc(void);
void key_press_cb(void);
#endif
