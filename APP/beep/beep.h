#ifndef _beep_H
#define _beep_H

#include "system.h"
#include "usart3.h"


/*  蜂鸣器时钟端口、引脚定义 */
#define BEEP_PORT 			GPIOB   
#define BEEP_PIN 			GPIO_Pin_8
#define BEEP_PORT_RCC		RCC_APB2Periph_GPIOB

#define BEEP PBout(8)

#define BEAT_FREQ   2               //节拍频率
#define BEAT_TIME   1000000/2       //节拍周期 us

void BEEP_Init(void);
void music_sample(void); 
void play_music1(void);              // 新增：两只老虎
void play_music2(void);              // 新增：打上花火片段

#endif
