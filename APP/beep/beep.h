#ifndef _beep_H
#define _beep_H

#include "system.h"
#include "usart3.h"


/*  ������ʱ�Ӷ˿ڡ����Ŷ��� */
#define BEEP_PORT 			GPIOB   
#define BEEP_PIN 			GPIO_Pin_8
#define BEEP_PORT_RCC		RCC_APB2Periph_GPIOB

#define BEEP PBout(8)

#define BEAT_FREQ   2               //����Ƶ��
#define BEAT_TIME   1000000/2       //�������� us

void BEEP_Init(void);
void music_sample(void); 
void play_music1(void);              // ��������ֻ�ϻ�
void play_music2(void);              // ���������ϻ���Ƭ��

#endif
