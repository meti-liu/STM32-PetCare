#include "pet_care.h"
#include "SysTick.h"
#include "tftlcd.h"
#include "usart.h"
#include "string.h"

// 全局变量定义
PetCare_TypeDef pet_care_data;

/**
 * @brief 初始化宠物照护系�?
 */
void PetCare_Init(void)
{
    // 初始化DHT11温湿度传感器
    while(DHT11_Init())
    {
        printf("DHT11 Error!\r\n");
        LCD_ShowString(10, 90, 200, 16, 16, "DHT11 Error!");
        delay_ms(500);
    }
    printf("DHT11 OK!\r\n");
    
    // 初始化光敏传感器
    Lsens_Init();
    
    // 初始化PWM控制(风扇)
    TIM3_CH3_PWM_Init(100, 7200-1); // 10KHz PWM频率，周期为100
    
    // 初始化RGB彩灯
    RGB_LED_Init();
    RGB_LED_Clear();
    
    // 初始化系统参�?
    pet_care_data.temperature = 25.0;
    pet_care_data.humidity = 50;
    pet_care_data.light_value = 50;
    pet_care_data.system_status = PET_CARE_NORMAL;
    pet_care_data.fan_status = DEVICE_OFF;
    pet_care_data.light_status = DEVICE_OFF;
    pet_care_data.beep_status = DEVICE_OFF;
    pet_care_data.auto_control = DEVICE_ON; // 默认开启自动控�?
    
    // 关闭所有设�?
    PetCare_Set_Fan(DEVICE_OFF);
    PetCare_Set_Light(DEVICE_OFF);
    PetCare_Set_Beep(DEVICE_OFF);
    
    printf("Pet Care System Initialized!\r\n");
}

/**
 * @brief 更新环境数据
 */
void PetCare_Update_Data(void)
{
    // 读取温湿�?
    u8 temperature = 0;
    u8 humidity = 0;
    if(DHT11_Read_Data(&temperature, &humidity) == 0) // 读取成功
    {
        pet_care_data.temperature = (float)temperature;
        pet_care_data.humidity = humidity;
    }
    
    // 读取光照�?
    pet_care_data.light_value = Lsens_Get_Val();
    
    // 更新系统状�?
    if(pet_care_data.temperature >= TEMP_ALARM_HIGH || 
       pet_care_data.temperature <= TEMP_ALARM_LOW || 
       pet_care_data.light_value <= LIGHT_ALARM_LOW)
    {
        pet_care_data.system_status = PET_CARE_ALARM;
    }
    else if(pet_care_data.temperature >= TEMP_WARNING_HIGH || 
            pet_care_data.temperature <= TEMP_WARNING_LOW || 
            pet_care_data.light_value <= LIGHT_WARNING_LOW)
    {
        pet_care_data.system_status = PET_CARE_WARNING;
    }
    else
    {
        pet_care_data.system_status = PET_CARE_NORMAL;
    }
    
    // 打印调试信息
    /*printf("Temperature: %.1f, Humidity: %d%%, Light: %d, Status: %d\r\n", 
           pet_care_data.temperature, 
           pet_care_data.humidity,
           pet_care_data.light_value, 
           pet_care_data.system_status);*/
}

/**
 * @brief 自动控制设备
 */
void PetCare_Auto_Control(void)
{
    // 温度控制 - 风扇
    u8 speed = 50; // 默认速度50%
		
		// 只有在自动控制模式下才执�?
    if(pet_care_data.auto_control != DEVICE_ON)
    {
        return;
    }
    
    
    
    if(pet_care_data.temperature >= TEMP_WARNING_HIGH)
    {
        // 温度过高，开启风�?
        PetCare_Set_Fan(DEVICE_ON);
        
        // 根据温度调整风扇速度
        if(pet_care_data.temperature >= TEMP_ALARM_HIGH)
        {
            speed = 100; // 温度过高，全速运�?
        }
        else
        {
            // 根据温度线性调整速度
            speed = 50 + (pet_care_data.temperature - TEMP_WARNING_HIGH) * 50 / (TEMP_ALARM_HIGH - TEMP_WARNING_HIGH);
            if(speed > 100) speed = 100;
        }
        
        TIM_SetCompare4(TIM4, speed);
    }
    else
    {
        // 温度正常，关闭风�?
        PetCare_Set_Fan(DEVICE_OFF);
    }
    
    // 光照控制 - RGB�?
    if(pet_care_data.light_value <= LIGHT_WARNING_LOW)
    {
        // 光照不足，开启照�?
        PetCare_Set_Light(DEVICE_ON);
    }
    else
    {
        // 光照充足，关闭照�?
        PetCare_Set_Light(DEVICE_OFF);
    }
    
    // 报警控制 - 蜂鸣�?
    if(pet_care_data.system_status == PET_CARE_ALARM)
    {
        // 报警状态，开启蜂鸣器
        PetCare_Set_Beep(DEVICE_ON);
    }
    else
    {
        // 非报警状态，关闭蜂鸣�?
        PetCare_Set_Beep(DEVICE_OFF);
    }
}


/**
 * @brief 显示环境数据
 */
void PetCare_Display_Data(void)
{
    char buf[50];
    
    // 显示温度
    sprintf(buf, "Temperature: %.1f C  ", pet_care_data.temperature);
    LCD_ShowString(10, 50, 200, 16, 16, (u8*)buf);
    
    // 显示湿度
    sprintf(buf, "Humidity: %d%%  ", pet_care_data.humidity);
    LCD_ShowString(10, 70, 200, 16, 16, (u8*)buf);
    
    // 显示光照
    sprintf(buf, "Light: %d%%  ", pet_care_data.light_value);
    LCD_ShowString(10, 90, 200, 16, 16, (u8*)buf);
    
    // 显示系统状�?
    switch(pet_care_data.system_status)
    {
        case PET_CARE_NORMAL:
            FRONT_COLOR = GREEN;
            LCD_ShowString(10, 110, 200, 16, 16, "Status: Normal  ");
            break;
        case PET_CARE_WARNING:
            FRONT_COLOR = YELLOW;
            LCD_ShowString(10, 110, 200, 16, 16, "Status: Warning  ");
            break;
        case PET_CARE_ALARM:
            FRONT_COLOR = RED;
            LCD_ShowString(10, 110, 200, 16, 16, "Status: Alarm!  ");
            break;
        default:
            break;
    }
    FRONT_COLOR = BLUE;
    
    // 显示设备状�?
    sprintf(buf, "Fan: %s  ", pet_care_data.fan_status == DEVICE_ON ? "ON" : "OFF");
    LCD_ShowString(10, 130, 200, 16, 16, (u8*)buf);
    
    sprintf(buf, "Light: %s  ", pet_care_data.light_status == DEVICE_ON ? "ON" : "OFF");
    LCD_ShowString(10, 150, 200, 16, 16, (u8*)buf);
    
    sprintf(buf, "Beep: %s  ", pet_care_data.beep_status == DEVICE_ON ? "ON" : "OFF");
    LCD_ShowString(10, 170, 200, 16, 16, (u8*)buf);
    
    sprintf(buf, "Auto: %s  ", pet_care_data.auto_control == DEVICE_ON ? "ON" : "OFF");
    LCD_ShowString(10, 190, 200, 16, 16, (u8*)buf);
}

/**
 * @brief 设置风扇状�?
 * @param status: DEVICE_ON/DEVICE_OFF
 */
void PetCare_Set_Fan(u8 status)
{
    if(status == DEVICE_ON)
    {
        TIM_SetCompare4(TIM4, 50); // 默认50%速度
        pet_care_data.fan_status = DEVICE_ON;
    }
    else
    {
        TIM_SetCompare4(TIM4, 0); // 停止
        pet_care_data.fan_status = DEVICE_OFF;
    }
}

/**
 * @brief 设置照明状�?
 * @param status: DEVICE_ON/DEVICE_OFF
 */
/*void PetCare_Set_Light(u8 status)
{
    if(status == DEVICE_ON)
    {
        // ??????�?????�?�???
        u8 i, j;
        for(i=0; i<RGB_LED_YHIGH; i++)
        {
            for(j=0; j<RGB_LED_XWIDTH; j++)
            {
                RGB_LED_Write_24Bits(255, 255, 255); // �????? (RGB??��??)
            }
        }
        pet_care_data.light_status = DEVICE_ON;
    }
    else
    {
        RGB_LED_Clear(); // ???�?��?�??�
        pet_care_data.light_status = DEVICE_OFF;
    }
}*/

void PetCare_Set_Light(u8 status)
{
    if(status == DEVICE_ON)
    {
        u32 color;
			  u8 i,j;
        if(pet_care_data.light_value < 20)
            color = RGB_COLOR_BLUE;
        else if(pet_care_data.light_value < 40)
            color = RGB_COLOR_GREEN;
        else
            color = RGB_COLOR_WHITE;
				
        for(i = 0; i < RGB_LED_YHIGH; i++)
        {
            for(j = 0; j < RGB_LED_XWIDTH; j++)
            {
                RGB_LED_Write_24Bits((color >> 16) & 0xFF,  // Green
                                     (color >> 8)  & 0xFF,  // Red
                                     (color)       & 0xFF); // Blue
            }
        }	*/	

					RGB_DrawDotColor(2, 2, 1, color);
	

        pet_care_data.light_status = DEVICE_ON;
    }
    else
    {
        RGB_LED_Clear(); // 关闭照明
        pet_care_data.light_status = DEVICE_OFF;
    }
}







/**
 * @brief 设置蜂鸣器状�?
 * @param status: DEVICE_ON/DEVICE_OFF
 */
void PetCare_Set_Beep(u8 status)
{
    if(status == DEVICE_ON)
    {
        BEEP = 1; // 开启蜂鸣器
        pet_care_data.beep_status = DEVICE_ON;
    }
    else
    {
        BEEP = 0; // 关闭蜂鸣�?
        pet_care_data.beep_status = DEVICE_OFF;
    }
}

/**
 * @brief 设置自动控制状�?
 * @param status: DEVICE_ON/DEVICE_OFF
 */
void PetCare_Set_Auto_Control(u8 status)
{
    pet_care_data.auto_control = status;
    
    if(status != DEVICE_ON)
    {
        // 关闭自动控制时，默认关闭所有设�?
        PetCare_Set_Fan(DEVICE_OFF);
        PetCare_Set_Light(DEVICE_OFF);
        PetCare_Set_Beep(DEVICE_OFF);
    }
}

/**
 * @brief 处理蓝牙命令
 * @param cmd: 命令字符�?
 */
void PetCare_Process_Command(char* cmd)
{
    printf("Received command: %s\r\n", cmd);
    
    // 风扇控制命令
    if(strcmp(cmd, "+FAN ON\r\n") == 0)
    {
        PetCare_Set_Auto_Control(DEVICE_OFF); // 关闭自动控制
        PetCare_Set_Fan(DEVICE_ON);
        printf("Fan turned ON\r\n");
    }
    else if(strcmp(cmd, "+FAN OFF\r\n") == 0)
    {
        PetCare_Set_Auto_Control(DEVICE_OFF); // 关闭自动控制
        PetCare_Set_Fan(DEVICE_OFF);
        printf("Fan turned OFF\r\n");
    }
    
    // 照明控制命令
    else if(strcmp(cmd, "+LIGHT ON\r\n") == 0)
    {
        PetCare_Set_Auto_Control(DEVICE_OFF); // 关闭自动控制
        PetCare_Set_Light(DEVICE_ON);
        printf("Light turned ON\r\n");
    }
    else if(strcmp(cmd, "+LIGHT OFF\r\n") == 0)
    {
        PetCare_Set_Auto_Control(DEVICE_OFF); // 关闭自动控制
        PetCare_Set_Light(DEVICE_OFF);
        printf("Light turned OFF\r\n");
    }
    
    // 蜂鸣器控制命�?
    else if(strcmp(cmd, "+BEEP ON\r\n") == 0)
    {
        PetCare_Set_Auto_Control(DEVICE_OFF); // 关闭自动控制
        PetCare_Set_Beep(DEVICE_ON);
        printf("Beep turned ON\r\n");
    }
    else if(strcmp(cmd, "+BEEP OFF\r\n") == 0)
    {
        PetCare_Set_Auto_Control(DEVICE_OFF); // 关闭自动控制
        PetCare_Set_Beep(DEVICE_OFF);
        printf("Beep turned OFF\r\n");
    }
    
    // 自动控制命令
    else if(strcmp(cmd, "+AUTO ON\r\n") == 0)
    {
        PetCare_Set_Auto_Control(DEVICE_ON);
        printf("Auto control turned ON\r\n");
    }
    else if(strcmp(cmd, "+AUTO OFF\r\n") == 0)
    {
        PetCare_Set_Auto_Control(DEVICE_OFF);
        printf("Auto control turned OFF\r\n");
    }
    
    // 查询状态命�?
    else if(strcmp(cmd, "+STATUS?\r\n") == 0)
    {
        printf("Temperature: %.1f C\r\n", pet_care_data.temperature);
        printf("Humidity: %d%%\r\n", pet_care_data.humidity);
        printf("Light: %d%%\r\n", pet_care_data.light_value);
        printf("System Status: %d\r\n", pet_care_data.system_status);
        printf("Fan: %s\r\n", pet_care_data.fan_status == DEVICE_ON ? "ON" : "OFF");
        printf("Light: %s\r\n", pet_care_data.light_status == DEVICE_ON ? "ON" : "OFF");
        printf("Beep: %s\r\n", pet_care_data.beep_status == DEVICE_ON ? "ON" : "OFF");
        printf("Auto Control: %s\r\n", pet_care_data.auto_control == DEVICE_ON ? "ON" : "OFF");
    }

    if(strcmp(cmd,"+music on\r\n")==0)
    {
        printf("music play\r\n");
        music_sample();
    }
}