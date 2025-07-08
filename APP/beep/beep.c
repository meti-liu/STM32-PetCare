#include "beep.h"


/*******************************************************************************
* 函 数 名         : BEEP_Init
* 函数功能		   : 蜂鸣器初始化
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
void BEEP_Init(void)	  //端口初始化
{
	GPIO_InitTypeDef GPIO_InitStructure;	//声明一个结构体变量，用来初始化GPIO

	RCC_APB2PeriphClockCmd(BEEP_PORT_RCC,ENABLE);   /* 开启GPIO时钟 */

	/*  配置GPIO的模式和IO口 */
	GPIO_InitStructure.GPIO_Pin=BEEP_PIN;		//选择你要设置的IO口
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;		  //设置推挽输出模式
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;	  //设置传输速率
	GPIO_Init(BEEP_PORT,&GPIO_InitStructure); 	 /* 初始化GPIO */
	
	GPIO_ResetBits(BEEP_PORT,BEEP_PIN);
}


uint16_t tone[] = {
        0, 261, 294, 330, 349, 392, 440, 494
};

#define MUSIC_LEN   48     

uint8_t music[] = {             //简朴码   《一闪一闪亮晶晶》
    1,1,5,5,6,6,5,0,
    4,4,3,3,2,2,1,0,
    5,5,4,4,3,3,2,0,
    5,5,4,4,3,3,2,0,
    1,1,5,5,6,6,5,0,
    4,4,3,3,2,2,1,0,
};



static void beep_play(uint32_t freq)
{
    uint32_t cycle = 1000000/freq;
    BEEP = 1;
    delay_us(cycle/2);
    BEEP = 0;
    delay_us(cycle/2);
}

void beep_play_one_beat(uint32_t freq)              //发出一个节拍的声音
{
    uint32_t i = 0;
    uint32_t count = 0;

    if(freq == 0)
    {
        delay_us(BEAT_TIME);                        //停顿一个节拍
    } else {
        count = BEAT_TIME / (1000000 / freq);
        for(i = 0; i < count; i++)
        {
            beep_play(freq);
        }
    }
}

void music_sample(void)                             //音乐演奏
{
    uint16_t i = 0;
    for(i = 0; i < MUSIC_LEN; i++)
    {
        beep_play_one_beat(tone[music[i]]);
        delay_us(BEAT_TIME/50);                     //短停顿
    }
}