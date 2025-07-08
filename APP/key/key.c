#include "key.h"
#include "SysTick.h"

/*******************************************************************************
* �� �� ��         : KEY_Init
* ��������		   : ������ʼ��
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/
void KEY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; //����ṹ�����	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOE,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin=KEY_UP_PIN;	   //ѡ����Ҫ���õ�IO��
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPD;//��������  
	GPIO_Init(KEY_UP_PORT,&GPIO_InitStructure);		  /* ��ʼ��GPIO */
	
	GPIO_InitStructure.GPIO_Pin=KEY0_PIN|KEY1_PIN|KEY2_PIN;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;	//��������
	GPIO_Init(KEY_PORT,&GPIO_InitStructure);
}

/*******************************************************************************
* �� �� ��         : KEY_Scan
* ��������		   : ����ɨ����
* ��    ��         : mode=0:���ΰ��°���
					 mode=1���������°���
* ��    ��         : 0��δ�а�������
					 KEY_UP_PRESS��KEY_UP������
					 KEY0_PRESS��KEY0������
					 KEY1_PRESS��KEY1������
*******************************************************************************/
u8 KEY_Scan(u8 mode)
{
	static u8 key=1;
	
	if(mode==1) //支持连按
		key=1;
	if(key==1&&(KEY_UP==1||KEY0==0||KEY1==0||KEY2==0)) //有按键按下
	{
		delay_ms(10);  //去抖
		key=0;
		if(KEY_UP==1)
			return KEY_UP_PRESS; 
		else if(KEY0==0)
			return KEY0_PRESS; 
		else if(KEY1==0)
			return KEY1_PRESS; 
		else if(KEY2==0)
			return KEY2_PRESS; 
	}
	else if(KEY_UP==0&&KEY0==1&&KEY1==1&&KEY2==1)    //无按键按下
		key=1;
	return 0;
}

