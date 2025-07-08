#include "pwm.h"

/*******************************************************************************
* �� �� ��         : TIM3_CH3_PWM_Init
* ��������		   : TIM3ͳ��3 PWM��ʼ������
* ��    ��         : per:��װ��ֵ
					 psc:��Ƶϵ��
* ��    ��         : ��
*******************************************************************************/
void TIM3_CH3_PWM_Init(u16 per,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;  // ��ʱ���������ýṹ��
    TIM_OCInitTypeDef TIM_OCInitStructure;              // ����Ƚ����ýṹ��
    GPIO_InitTypeDef GPIO_InitStructure;                // GPIO���ýṹ��

    /* ��������ʱ�� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  // ʹ��GPIOBʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);   // ʹ��TIM4ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);   // ʹ�ܸ��ù���ʱ��

    /* ����PB9ΪTIM4ͨ��4������� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;          // ѡ��PB9���ţ���׼TIM4_CH4ӳ�䣩
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // GPIO�������ٶ�50MHz
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;    // �����������ģʽ
    GPIO_Init(GPIOB, &GPIO_InitStructure);             // ��ʼ��PB9

    /* ����TIM4�������� */
    TIM_TimeBaseInitStructure.TIM_Period = per;             // �Զ���װ��ֵ
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;          // Ԥ��Ƶϵ��
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; // ʱ�Ӳ���Ƶ
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; // ���ϼ���ģʽ
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);     // Ӧ��TIM4����

    /* ����TIM4ͨ��4ΪPWM��� */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;       // PWMģʽ1
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // �ߵ�ƽ��Ч
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // ʹ��ͨ�����
    TIM_OC4Init(TIM4, &TIM_OCInitStructure);              // Ӧ�����õ�ͨ��4

    /* ʹ�ܼĴ���Ԥװ�� */
    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);     // ʹ�ܱȽ�ֵԤװ��
    TIM_ARRPreloadConfig(TIM4, ENABLE);                   // ʹ������ֵԤװ��

    /* ������ʱ�� */
    TIM_Cmd(TIM4, ENABLE);                                // ʹ��TIM4������
}


