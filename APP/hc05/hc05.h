#ifndef _hc05_H
#define _hc05_H	

#include "system.h" 


#define HC05_KEY  	PAout(4) 		//��������KEY�ź�
#define HC05_LED  	PAin(15)		//��������״̬�ź�
  
u8 HC05_Init(void);
u8 HC05_Get_Role(void);
u8 HC05_Set_Cmd(u8* atstr);	

#endif  
















