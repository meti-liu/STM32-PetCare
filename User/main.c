#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "key.h"
#include "usart.h"
#include "tftlcd.h"
#include "hc05.h"
#include "usart3.h"			 	 
#include "string.h"
#include "dht11.h"
#include "lsens.h"
#include "beep.h"
#include "pwm.h"
#include "ws2812.h"
#include "pet_care.h"

	

//???HC05??????????
void HC05_Role_Show(void)
{
	if(HC05_Get_Role()==1)
	{
		LCD_ShowString(10,140,200,16,16,"ROLE:Master");	//????
	}
	else 
	{
		LCD_ShowString(10,140,200,16,16,"ROLE:Slave ");	//???
	}
}

//???HC05??????????
void HC05_Sta_Show(void)
{												 
	if(HC05_LED)
	{
		LCD_ShowString(110,140,120,16,16,"STA:Connected ");	//??????
	}
	else 
	{
		LCD_ShowString(110,140,120,16,16,"STA:Disconnect");	 			//��????
	}				 
}


int main()
{
	u8 t=0;
	u8 key;
	u8 reclen=0; 
	
	// ϵͳ��ʼ��
	SysTick_Init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  // �ж����ȼ����� ��2
	LED_Init();
	KEY_Init();
	RGB_LED_Init();
	USART1_Init(115200);
	TFTLCD_Init();			// LCD��ʼ��
	BEEP_Init();			// ��������ʼ��
	TIM3_CH3_PWM_Init(100, 7200-1); // ����PWM��ʼ��
	
	// ��ʾ��ӭ����
	FRONT_COLOR=RED;
	LCD_ShowString(10,10,tftlcd_data.width,tftlcd_data.height,16,"Pet Care System");
	LCD_ShowString(10,30,tftlcd_data.width,tftlcd_data.height,16,"Smart Pet Guardian");
	delay_ms(1000);			// �ȴ�����ģ���ϵ��ȶ�
	
	// ��ʼ������ģ��
	while(HC05_Init()) 		// ��ʼ��HC05ģ��  
	{
		printf("HC05 Error!\r\n");
		LCD_ShowString(10,90,200,16,16,"HC05 Error!    "); 
		delay_ms(500);
		LCD_ShowString(10,90,200,16,16,"Please Check!!!"); 
		delay_ms(100);
	}
	printf("HC05 OK!\r\n");
	
	// �޸�����ģ������
	//HC05_Set_Cmd("AT+NAME=PetCare");
	//HC05_Set_Cmd("AT+RESET"); // ��������ģ��ʹ������Ч
	//delay_ms(200);
	
	// ��������ʾ������ʾ
	LCD_Clear(WHITE);
	FRONT_COLOR=RED;
	LCD_ShowString(10,10,tftlcd_data.width,tftlcd_data.height,16,"Pet Care System");
	LCD_ShowString(10,30,tftlcd_data.width,tftlcd_data.height,16,"Smart Pet Guardian");
	
	// ��ʾ����״̬
	FRONT_COLOR=BLUE;
	HC05_Role_Show();
	HC05_Sta_Show();
	
	// ��ʼ�������ջ�ϵͳ
	PetCare_Init();
	
	// ��ʾ��ʼ����
	PetCare_Display_Data();
	
	USART3_RX_STA=0;
 	while(1) 
	{		
		// ��������
		key=KEY_Scan(0);
		if(key==KEY_UP_PRESS)		// �л��Զ�����ģʽ
		{
			if(pet_care_data.auto_control == DEVICE_ON)
			{
				PetCare_Set_Auto_Control(DEVICE_OFF);
				printf("Auto control turned OFF\r\n");
			}
			else
			{
				PetCare_Set_Auto_Control(DEVICE_ON);
				printf("Auto control turned ON\r\n");
			}
		}
		else if(key==KEY1_PRESS)	// �ֶ����Ʒ���
		{
			if(pet_care_data.fan_status == DEVICE_ON)
			{
				PetCare_Set_Fan(DEVICE_OFF);
				printf("Fan turned OFF\r\n");
			}
			else
			{
				PetCare_Set_Auto_Control(DEVICE_OFF); // �ر��Զ�����
				PetCare_Set_Fan(DEVICE_ON);
				printf("Fan turned ON\r\n");
			}
		}
		
		// ��ʱ����
		if(t >= 50) // Լ500ms
		{
			// ���»�������
			PetCare_Update_Data();
			
			// �Զ�����
			PetCare_Auto_Control();
			
			// ������ʾ
			PetCare_Display_Data();
			
			// ��ʾ����״̬
			HC05_Sta_Show();
			
			// LEDָʾ����˸
			LED1=!LED1;
			
			t=0;
		}
		
		// ���������
		if(USART3_RX_STA&0X8000)		// ���յ�һ������
		{
 			reclen=USART3_RX_STA&0X7FFF;	// �õ����ݳ���
		  	USART3_RX_BUF[reclen]='\0';	 	// ��ӽ�����
			printf("Received: %s\r\n", USART3_RX_BUF);
			
			// ������������
			PetCare_Process_Command((char*)USART3_RX_BUF);
			
			// ������ʾ
			PetCare_Display_Data();
			
 			USART3_RX_STA=0;	 
		}
		
		delay_ms(10);
		t++;	
	}
}
