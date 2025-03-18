#include "timer2.h"
#include "stm32f10x.h"                  // Device header

//arr=1000.psc=72，溢出时间为1ms
void TIM2_Int_Init(uint16_t arr,uint16_t psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);               //时钟使能
    
    //定时器TIM3初始化
    TIM_TimeBaseStructure.TIM_Period = arr;                             //设置在下一个更新事件装入活动的自动重装载寄存器周期的值   
    TIM_TimeBaseStructure.TIM_Prescaler =psc;                           //设置用来作为TIMx时钟频率除数的预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;           //设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;      //TIM向上计数模式
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);                   //根据指定的参数初始化TIMx的时间基数单位
    
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE );                          //使能指定的TIM3中断,允许更新中断
    
    //中断优先级NVIC设
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;                   //TIM3中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;        //先占优先级0级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;                //从优先级3级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                   //IRQ通道被使能
    NVIC_Init(&NVIC_InitStructure);                                    //初始化NVIC寄存器
    
    TIM_Cmd(TIM2, ENABLE);                                              //使能TIMx    
}












