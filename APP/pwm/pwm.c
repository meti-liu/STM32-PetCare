#include "pwm.h"

/*******************************************************************************
* 锟斤拷 锟斤拷 锟斤拷         : TIM3_CH3_PWM_Init
* 锟斤拷锟斤拷锟斤拷锟斤拷		   : TIM3统锟斤拷3 PWM锟斤拷始锟斤拷锟斤拷锟斤拷
* 锟斤拷    锟斤拷         : per:锟斤拷装锟斤拷值
					 psc:锟斤拷频系锟斤拷
* 锟斤拷    锟斤拷         : 锟斤拷
*******************************************************************************/
void TIM3_CH3_PWM_Init(u16 per,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;  // 定时器基础配置结构体
    TIM_OCInitTypeDef TIM_OCInitStructure;              // 输出比较配置结构体
    GPIO_InitTypeDef GPIO_InitStructure;                // GPIO配置结构体

    /* 启动外设时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  // 使能GPIOB时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);   // 使能TIM4时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);   // 使能复用功能时钟

    /* 配置PB9为TIM4通道4输出引脚 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;          // 选择PB9引脚（标准TIM4_CH4映射）
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // GPIO最高输出速度50MHz
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;    // 复用推挽输出模式
    GPIO_Init(GPIOB, &GPIO_InitStructure);             // 初始化PB9

    /* 配置TIM4基础参数 */
    TIM_TimeBaseInitStructure.TIM_Period = per;             // 自动重装载值
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;          // 预分频系数
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; // 时钟不分频
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);     // 应用TIM4配置

    /* 配置TIM4通道4为PWM输出 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;       // PWM模式1
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // 高电平有效
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 使能通道输出
    TIM_OC4Init(TIM4, &TIM_OCInitStructure);              // 应用配置到通道4

    /* 使能寄存器预装载 */
    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);     // 使能比较值预装载
    TIM_ARRPreloadConfig(TIM4, ENABLE);                   // 使能周期值预装载

    /* 启动定时器 */
    TIM_Cmd(TIM4, ENABLE);                                // 使能TIM4计数器
}


