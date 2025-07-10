#include "beep.h"
#include "stdio.h"
#include <string.h>
#include "SysTick.h"

/*******************************************************************************
* �� �� ��         : BEEP_Init
* ��������		   : ��������ʼ��
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/
void BEEP_Init(void)	  //�˿ڳ�ʼ��
{
	GPIO_InitTypeDef GPIO_InitStructure;	//����һ���ṹ�������������ʼ��GPIO

	RCC_APB2PeriphClockCmd(BEEP_PORT_RCC,ENABLE);   /* ����GPIOʱ�� */

	/*  ����GPIO��ģʽ��IO�� */
	GPIO_InitStructure.GPIO_Pin=BEEP_PIN;		//ѡ����Ҫ���õ�IO��
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;		  //�����������ģʽ
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;	  //���ô�������
	GPIO_Init(BEEP_PORT,&GPIO_InitStructure); 	 /* ��ʼ��GPIO */
	
	GPIO_ResetBits(BEEP_PORT,BEEP_PIN);
}

extern uint8_t music_mode;  // ȷ���ܷ���ȫ�ֱ���




#define H(n)  ((n) + 10)   /* �߰˶� */
#define L(n)  ((n) + 20)   /* �Ͱ˶� */

static const uint16_t tone[] = {
    /* 0 */   0,          /* ��ֹ�� */
    /* 1-7  ���� C4~B4 */ 262,294,330,349,392,440,494,
    /* 8-10 Ԥ���������ţ����� 0 */
               0,   0,   0,
    /* 11-17 ���� C5~B5 */523,587,659,698,784,880,988,
    /* 18-20 Ԥ�� */
               0,   0,   0,
    /* 21-27 ���� C3~B3 */131,147,165,175,196,220,247
};
/* ------------------------------------------ */


#define MUSIC_LEN   48     

uint8_t music[] = {             //������   ��һ��һ����������
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

void beep_play_one_beat(uint32_t freq)              //����һ�����ĵ�����
{
    uint32_t i = 0;
    uint32_t count = 0;

    if(freq == 0)
    {
        delay_us(BEAT_TIME);                        //ͣ��һ������
    } else {
        count = BEAT_TIME / (1000000 / freq);
        for(i = 0; i < count; i++)
        {
            beep_play(freq);
        }
    }
}

void music_sample(void)                             //��������
{
    uint16_t i = 0;
    for(i = 0; i < MUSIC_LEN; i++)
    {
        beep_play_one_beat(tone[music[i]]);
        delay_us(BEAT_TIME/50);
		if(USART3_RX_STA&0X8000)		// ���յ�һ������
		{	
			u8 reclen;
 			reclen=USART3_RX_STA&0X7FFF;	// �õ����ݳ���
		  	USART3_RX_BUF[reclen]='\0';	 	// ��ӽ�����
			if(strcmp("+stop music\r\n",(char *)USART3_RX_BUF)==0){
				return;
			}
 			USART3_RX_STA=0;	 
		}                  //��ͣ��
    }
}



void play_music1(void)
{
    uint8_t notes[] = {
    12,13,15,11,12,13,15,11,12,13,15,16,12,13,6,11,
		2,3,5,1,2,3,5,1,2,3,5,6,2,3,26,1,
		3,5,6,0,11,12,0,7,5,4,0,3,5,7,11,
		11,7,6,11,0,7,5,5
    };
    uint16_t len = sizeof(notes) / sizeof(notes[0]);
    uint16_t i = 0;

    for (i = 0; i < len; i++)
    {
        beep_play_one_beat(tone[notes[i]]);
        delay_us(BEAT_TIME / 50);  // ģ���ͣ��

        if (USART3_RX_STA & 0x8000)
        {
            uint8_t reclen = USART3_RX_STA & 0x7FFF;
            USART3_RX_BUF[reclen] = '\0';
            if (strcmp("+stop music\r\n", (char *)USART3_RX_BUF) == 0)
            {
                USART3_RX_STA = 0;
							  music_mode = 0;  //�˳�����ģʽ
                return;
            }
            USART3_RX_STA = 0;
        }
    }
}


void play_music2(void)
{
		uint8_t notes[] = {
    3, 2, 4, 3, 1, 5, 7, H(1), 7, 5, 1, 
    1, 6, 6, 6, 5 ,5, 5, 4, 3, 2, 3, 4, 3,
    3,4,5,3,4,5,7, H(2),7,H(1),H(1),
		H(1),H(1),5,5,6,5,4,2,3,4,5,6,1,6,7,7
		
};

    uint16_t len = sizeof(notes) / sizeof(notes[0]);
    uint16_t i = 0;

    for (i = 0; i < len; i++)
    {
        beep_play_one_beat(tone[notes[i]]);
        delay_us(BEAT_TIME / 50);  // ģ���ͣ��

        if (USART3_RX_STA & 0x8000)
        {
            uint8_t reclen = USART3_RX_STA & 0x7FFF;
            USART3_RX_BUF[reclen] = '\0';
            if (strcmp("+stop music\r\n", (char *)USART3_RX_BUF) == 0)
            {
                USART3_RX_STA = 0;
							  music_mode = 0;  //�˳�����ģʽ
                return;
            }
            USART3_RX_STA = 0;
        }
    }
}
