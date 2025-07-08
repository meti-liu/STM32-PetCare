#include "beep.h"
#include "stdio.h"

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


uint16_t tone[] = {
        0, 261, 294, 330, 349, 392, 440, 494
};

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