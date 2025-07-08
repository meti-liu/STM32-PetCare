#ifndef _key_H
#define _key_H


#include "system.h"
 
#define KEY0_PIN   			GPIO_Pin_4    	//定义KEY0引脚
#define KEY1_PIN    		GPIO_Pin_3    	//定义KEY1引脚
#define KEY2_PIN    		GPIO_Pin_2    	//定义KEY2引脚
#define KEY_UP_PIN  		GPIO_Pin_0  	//定义KEY_UP引脚

#define KEY_PORT 			GPIOE 		//定义端口
#define KEY_UP_PORT 		GPIOA 		//定义端口

//使用位带操作定义
#define KEY_UP 	PAin(0)
#define KEY0 	PEin(4)
#define KEY1 	PEin(3)
#define KEY2 	PEin(2)

//按键返回值定义  
#define KEY_UP_PRESS 	1
#define KEY0_PRESS		2
#define KEY1_PRESS		3
#define KEY2_PRESS		4
 
void KEY_Init(void);
u8 KEY_Scan(u8 mode);

#endif
