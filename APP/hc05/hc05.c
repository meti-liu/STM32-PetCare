#include "SysTick.h"
#include "usart.h"
#include "usart3.h"
#include "hc05.h"
#include "led.h"
#include "string.h"
#include "math.h"

//��ʼ��HC05 ģ��
//����ֵ:0,�ɹ�;1,ʧ��.
u8 HC05_Init(void)
{
    u8 retry=10,t;
    u8 temp=1;

    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE); //ʹ��PORTA

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15; // �˿�����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //��������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO ���ٶ�Ϊ50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure); //�����趨������ʼ��A15

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; // �˿�����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO ���ٶ�Ϊ50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure); //�����趨������ʼ��GPIOA4

    GPIO_SetBits(GPIOA,GPIO_Pin_4);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
    HC05_KEY=1;
    HC05_LED=1;

    USART3_Init(9600); //��ʼ������2 Ϊ:9600,������.

    while(retry--)
    {
        HC05_KEY=1; //KEY �ø�,����AT ģʽ
        delay_ms(10);
        u3_printf("AT\r\n"); //����AT ����ָ��
        HC05_KEY=0; //KEY ����,�˳�AT ģʽ
        for(t=0;t<10;t++) //��ȴ�50ms,������HC05 ģ��Ļ�Ӧ
        {
            if(USART3_RX_STA&0X8000)break;
            delay_ms(5);
        }
        if(USART3_RX_STA&0X8000) //���յ�һ��������
        {
            temp=USART3_RX_STA&0X7FFF; //�õ����ݳ���
            USART3_RX_STA=0;
            if(temp==4&&USART3_RX_BUF[0]=='O'&&USART3_RX_BUF[1]=='K')
            {
                temp=0;//���յ�OK ��Ӧ
                break;
            }
        }
    }
    if(retry==0)temp=1; //���ʧ��
    return temp;
}

//��ȡATK-HC05 ģ��Ľ�ɫ
//����ֵ:0,�ӻ�;1,����;0XFF,��ȡʧ��.
u8 HC05_Get_Role(void)
{
    u8 retry=0X0F;
    u8 temp,t;
    while(retry--)
    {
        HC05_KEY=1; //KEY �ø�,����AT ģʽ
        delay_ms(10);
        u3_printf("AT+ROLE?\r\n"); //��ѯ��ɫ
        for(t=0;t<20;t++) //��ȴ�200ms,������HC05 ģ��Ļ�Ӧ
        {
            delay_ms(10);
            if(USART3_RX_STA&0X8000)break;
        }
        HC05_KEY=0; //KEY ����,�˳�AT ģʽ
        if(USART3_RX_STA&0X8000) //���յ�һ��������
        {
            temp=USART3_RX_STA&0X7FFF; //�õ����ݳ���
            USART3_RX_STA=0;
            if(temp==13&&USART3_RX_BUF[0]=='+')//���յ���ȷ��Ӧ����
            {
                temp=USART3_RX_BUF[6]-'0';//�õ�����ģʽֵ
                break;
            }
        }
    }
    if(retry==0)temp=0XFF;//��ѯʧ��.
    return temp;
}

//HC05 ��������
//�˺�����������HC05,�����ڽ�����OK Ӧ���AT ָ��
//atstr:AT ָ�.����:"AT+RESET"/"AT+UART=9600,0,0"/"AT+ROLE=0"���ַ���
//����ֵ:0,���óɹ�;����,����ʧ��.
u8 HC05_Set_Cmd(u8* atstr)
{
    u8 retry=0X0F;
    u8 temp,t;
    while(retry--)
    {
        HC05_KEY=1; //KEY �ø�,����AT ģʽ
        delay_ms(10);
        u3_printf("%s\r\n",atstr); //����AT �ַ���
        HC05_KEY=0; //KEY ����,�˳�AT ģʽ
        for(t=0;t<20;t++) //��ȴ�100ms,������HC05 ģ��Ļ�Ӧ
        {
            if(USART3_RX_STA&0X8000)break;
            delay_ms(5);
        }
        if(USART3_RX_STA&0X8000) //���յ�һ��������
        {
            temp=USART3_RX_STA&0X7FFF; //�õ����ݳ���
            USART3_RX_STA=0;
            if(temp==4&&USART3_RX_BUF[0]=='O')//���յ���ȷ��Ӧ����
            {
                temp=0;
                break;
            }
        }
    }
    if(retry==0)temp=0XFF;//����ʧ��.
    return temp;
}

//str:���.(����ע�ⲻ����Ҫ������س���)
void HC05_CFG_CMD(u8 *str)
{
    u8 temp;
    u8 t;
    HC05_KEY=1; //KEY �ø�,����AT ģʽ
    delay_ms(10);
    u3_printf("%s\r\n",(char*)str); //����ָ��
    for(t=0;t<50;t++) //��ȴ�500ms,������HC05 ģ��Ļ�Ӧ
    {
        if(USART3_RX_STA&0X8000)break;
        delay_ms(10);
    }
    HC05_KEY=0; //KEY ����,�˳�AT ģʽ
    if(USART3_RX_STA&0X8000) //���յ�һ��������
    {
        temp=USART3_RX_STA&0X7FFF; //�õ����ݳ���
        USART3_RX_STA=0;
        USART3_RX_BUF[temp]=0; //�ӽ�����
        printf("\r\n%s",USART3_RX_BUF);//���ͻ�Ӧ���ݵ�����1
    }
}
