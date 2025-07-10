#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "key.h"
#include "usart.h"
#include "tftlcd.h"
#include "hc05.h"
#include "usart3.h"			 	 
#include "usart2.h"			 	 
#include "string.h"
#include "dht11.h"
#include "lsens.h"
#include "beep.h"
#include "pwm.h"
#include "ws2812.h"
#include "pet_care.h"

	

// 不需要单独显示蓝牙角色和状态，已在 PetCare_Display_Page 中显示
/*
void HC05_Role_Show(void)
{
	if(HC05_Get_Role()==1)
	{
		LCD_ShowString(10,140,200,16,16,"ROLE:Master");	// 主机
	}
	else 
	{
		LCD_ShowString(10,140,200,16,16,"ROLE:Slave ");	// 从机
	}
}

void HC05_Sta_Show(void)
{												 
	if(HC05_LED)
	{
		LCD_ShowString(110,140,120,16,16,"STA:Connected ");	// 连接成功
	}
	else 
	{
		LCD_ShowString(110,140,120,16,16,"STA:Disconnect");	// 未连接
	}				 
}
*/


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
	//RGB_LED_Init();
	USART1_Init(115200);
	USART2_Init(9600);		// UART2初始化，用于与电脑通信
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
	
	// 清屏准备显示
	LCD_Clear(WHITE);
	
	// 不需要单独显示蓝牙角色和状态，将在 PetCare_Display_Page 中显示
	FRONT_COLOR=BLUE;
	
	// ��ʼ�������ջ�ϵͳ
	PetCare_Init();
	
	// ��ʾ��ʼ����
	PetCare_Display_Data();
	
	USART3_RX_STA=0;
 	while(1) 
	{		
		// 按键扫描
		key=KEY_Scan(0);
		if(key==KEY0_PRESS)		// KEY0按下，切换页面
		{
			PetCare_Switch_Page();
			printf("Page switched\r\n");
		}
		else if(key==KEY_UP_PRESS)	// 切换自动控制模式
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
		else if(key==KEY1_PRESS)	// 手动控制风扇
		{
			if(pet_care_data.fan_status == DEVICE_ON)
			{
				PetCare_Set_Fan(DEVICE_OFF);
				printf("Fan turned OFF\r\n");
			}
			else
			{
				PetCare_Set_Auto_Control(DEVICE_OFF); // 关闭自动控制
				PetCare_Set_Fan(DEVICE_ON);
				printf("Fan turned ON\r\n");
			}
		}
		
		// 定时任务
		if(t >= 50) // 约500ms
		{
			// 更新环境数据
			PetCare_Update_Data();
			
			// 自动控制
			PetCare_Auto_Control();
			
			// LED指示灯闪烁
			LED1=!LED1;
			
			t=0;
		}
		
		// ���������
		if(USART3_RX_STA&0X8000)		// ���յ�һ������
		{
 			reclen=USART3_RX_STA&0X7FFF;	// �õ����ݳ���
		  	USART3_RX_BUF[reclen]='\0';	 	// ���ӽ�����
			printf("Received: %s\r\n", USART3_RX_BUF);
			
			// 处理蓝牙命令
		// 检查是否是DeepSeek相关命令
		if(strncmp((char*)USART3_RX_BUF, "+deepseek", 9) == 0 || 
		   strncmp((char*)USART3_RX_BUF, "+deepseek_response", 18) == 0)
		{
			// DeepSeek相关命令转发到UART2处理
			PetCare_Process_Command_Ex((char*)USART3_RX_BUF, 1);
		}
		else
		{
			// 其他命令保持原有处理方式
			PetCare_Process_Command((char*)USART3_RX_BUF);
			// 更新显示
			PetCare_Display_Data();
		}
 			USART3_RX_STA=0;	 
	}
	
	// 处理UART2数据（与电脑通信）
	if(USART2_RX_STA&0X8000)		// 接收到一批数据
	{
		reclen=USART2_RX_STA&0X7FFF;	// 得到数据长度
		USART2_RX_BUF[reclen]='\0';	 	// 添加结束符
		printf("UART2 Received: %s\r\n", USART2_RX_BUF);
		
		// 处理DeepSeek相关命令
		if(strncmp((char*)USART2_RX_BUF, "+deepseek", 9) == 0 || 
		   strncmp((char*)USART2_RX_BUF, "+deepseek_response", 18) == 0)
		{
			// DeepSeek相关命令始终使用UART2处理
			PetCare_Process_Command_Ex((char*)USART2_RX_BUF, 1);
		}
		else
		{
			// 其他命令保持原有处理方式
			PetCare_Process_Command((char*)USART2_RX_BUF);
			// 更新显示
			PetCare_Display_Data();
		}
		USART2_RX_STA=0;	 
	}
	
	delay_ms(10);
	t++;	
}
}
