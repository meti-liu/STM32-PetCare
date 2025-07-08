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
		LCD_ShowString(110,140,120,16,16,"STA:Disconnect");	 			//δ????
	}				 
}


int main()
{
	u8 t=0;
	u8 key;
	u8 reclen=0; 
	
	// 系统初始化
	SysTick_Init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  // 中断优先级分组 组2
	LED_Init();
	KEY_Init();
	RGB_LED_Init();
	USART1_Init(115200);
	TFTLCD_Init();			// LCD初始化
	BEEP_Init();			// 蜂鸣器初始化
	TIM3_CH3_PWM_Init(100, 7200-1); // 风扇PWM初始化
	
	// 显示欢迎界面
	FRONT_COLOR=RED;
	LCD_ShowString(10,10,tftlcd_data.width,tftlcd_data.height,16,"Pet Care System");
	LCD_ShowString(10,30,tftlcd_data.width,tftlcd_data.height,16,"Smart Pet Guardian");
	delay_ms(1000);			// 等待蓝牙模块上电稳定
	
	// 初始化蓝牙模块
	while(HC05_Init()) 		// 初始化HC05模块  
	{
		printf("HC05 Error!\r\n");
		LCD_ShowString(10,90,200,16,16,"HC05 Error!    "); 
		delay_ms(500);
		LCD_ShowString(10,90,200,16,16,"Please Check!!!"); 
		delay_ms(100);
	}
	printf("HC05 OK!\r\n");
	
	// 修改蓝牙模块名称
	//HC05_Set_Cmd("AT+NAME=PetCare");
	//HC05_Set_Cmd("AT+RESET"); // 重置蓝牙模块使名称生效
	//delay_ms(200);
	
	// 清屏并显示操作提示
	LCD_Clear(WHITE);
	FRONT_COLOR=RED;
	LCD_ShowString(10,10,tftlcd_data.width,tftlcd_data.height,16,"Pet Care System");
	LCD_ShowString(10,30,tftlcd_data.width,tftlcd_data.height,16,"Smart Pet Guardian");
	
	// 显示蓝牙状态
	FRONT_COLOR=BLUE;
	HC05_Role_Show();
	HC05_Sta_Show();
	
	// 初始化宠物照护系统
	PetCare_Init();
	
	// 显示初始数据
	PetCare_Display_Data();
	
	USART3_RX_STA=0;
 	while(1) 
	{		
		// 按键处理
		key=KEY_Scan(0);
		if(key==KEY_UP_PRESS)		// 切换自动控制模式
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
			
			// 更新显示
			PetCare_Display_Data();
			
			// 显示蓝牙状态
			HC05_Sta_Show();
			
			// LED指示灯闪烁
			LED1=!LED1;
			
			t=0;
		}
		
		// 蓝牙命令处理
		if(USART3_RX_STA&0X8000)		// 接收到一条数据
		{
 			reclen=USART3_RX_STA&0X7FFF;	// 得到数据长度
		  	USART3_RX_BUF[reclen]='\0';	 	// 添加结束符
			printf("Received: %s\r\n", USART3_RX_BUF);
			
			// 处理蓝牙命令
			PetCare_Process_Command((char*)USART3_RX_BUF);
			
			// 更新显示
			PetCare_Display_Data();
			
 			USART3_RX_STA=0;	 
		}
		
		delay_ms(10);
		t++;	
	}
}
