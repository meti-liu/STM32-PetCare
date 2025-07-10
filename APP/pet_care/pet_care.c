#include "pet_care.h"
#include "SysTick.h"
#include "tftlcd.h"
#include "usart.h"
#include "string.h"
#include "doro2.h"
#include "doro_hot.h"
#include "doro_unhappy.h"
#include "key.h"
#include "usart3.h"  // 添加usart3.h头文件，以便访问USART3_RX_BUF变量
#include "usart2.h"  // 添加usart2.h头文件，以便访问USART2相关功能

// 全局变量定义
PetCare_TypeDef pet_care_data;

/**
 * @brief 初始化宠物照护系�??
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
    
    // 初始化系统参�??
    pet_care_data.temperature = 25.0;
    pet_care_data.humidity = 50;
    pet_care_data.light_value = 50;
    pet_care_data.system_status = PET_CARE_NORMAL;
    pet_care_data.fan_status = DEVICE_OFF;
    pet_care_data.light_status = DEVICE_OFF;
    pet_care_data.beep_status = DEVICE_OFF;
    pet_care_data.auto_control = DEVICE_ON; // 默认开启自动控�??
    pet_care_data.current_page = PAGE_ROLE_STATUS; // 默认显示角色和状态页�??
    
    // 关闭所有设�????
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
    // 保存旧数据用于比�??
    float old_temperature = pet_care_data.temperature;
    u8 old_humidity = pet_care_data.humidity;
    u8 old_light_value = pet_care_data.light_value;
    u8 old_system_status = pet_care_data.system_status;
    
    // 读取温湿�??
    u8 temperature = 0;
    u8 humidity = 0;
    if(DHT11_Read_Data(&temperature, &humidity) == 0) // 读取成功
    {
        pet_care_data.temperature = (float)temperature;
        pet_care_data.humidity = humidity;
        
        // 添加调试信息
        printf("Temperature: %.1f, Humidity: %d%%\r\n", pet_care_data.temperature, pet_care_data.humidity);
        
        // 检查温度是否变化，如果变化则更新温度显�??
        if(pet_care_data.temperature != old_temperature)
        {
            PetCare_Update_Display_Item(0); // 更新温度显示
            
            // 如果当前是图片页面，且温度变化可能导致图片变化，则更新图片显�??
            if(pet_care_data.current_page == PAGE_PICTURE && 
              ((old_temperature < 28.0 && pet_care_data.temperature >= 28.0) || 
               (old_temperature >= 28.0 && pet_care_data.temperature < 28.0)))
            {
                printf("Temperature crossed threshold (%.1f -> %.1f), updating picture page\r\n", old_temperature, pet_care_data.temperature);
                PetCare_Display_Data(); // 更新整个页面
            }
        }
        
        // 检查湿度是否变化，如果变化则更新湿度显�??
        if(pet_care_data.humidity != old_humidity)
        {
            PetCare_Update_Display_Item(1); // 更新湿度显示
        }
    }
    
    // 读取光照�??
    pet_care_data.light_value = Lsens_Get_Val();
    
    // 添加调试信息
    printf("Light value: %d\r\n", pet_care_data.light_value);
    
    // 检查光照值是否变化，如果变化则更新光照显�??
    if(pet_care_data.light_value != old_light_value)
    {
        PetCare_Update_Display_Item(2); // 更新光照显示
        
        // 如果当前是图片页面，且光照值变化可能导致图片变化，则更新图片显�??
        if(pet_care_data.current_page == PAGE_PICTURE && 
           ((old_light_value <= 40 && pet_care_data.light_value > 40) || 
            (old_light_value > 40 && pet_care_data.light_value <= 40)))
        {
            printf("Light value crossed threshold (%d -> %d), updating picture page\r\n", old_light_value, pet_care_data.light_value);
            PetCare_Display_Data(); // 更新整个页面
        }
    }
    
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
    
    // 检查系统状态是否变化，如果变化则更新系统状态显�??
    if(pet_care_data.system_status != old_system_status)
    {
        PetCare_Update_Display_Item(7); // 更新系统状态显�??
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
		
		// 只有在自动控制模式下才执�????
    if(pet_care_data.auto_control != DEVICE_ON)
    {
        return;
    }
    
    
    
    if(pet_care_data.temperature >= TEMP_WARNING_HIGH)
    {
        // 温度过高，开启风�????
        PetCare_Set_Fan(DEVICE_ON);
        
        // 根据温度调整风扇速度
        if(pet_care_data.temperature >= TEMP_ALARM_HIGH)
        {
            speed = 100; // 温度过高，全速运�????
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
        // 温度正常，关闭风�????
        PetCare_Set_Fan(DEVICE_OFF);
    }
    
    // 光照控制 - RGB�????
    if(pet_care_data.light_value <= LIGHT_WARNING_LOW)
    {
        // 光照不足，开启照�????
        PetCare_Set_Light(DEVICE_ON);
    }
    else
    {
        // 光照充足，关闭照�????
        PetCare_Set_Light(DEVICE_OFF);
    }
    
    // 报警控制 - 蜂鸣�????
    if(pet_care_data.system_status == PET_CARE_ALARM)
    {
        // 报警状态，开启蜂鸣器
        PetCare_Set_Beep(DEVICE_ON);
    }
    else
    {
        // 非报警状态，关闭蜂鸣�????
        PetCare_Set_Beep(DEVICE_OFF);
    }
}

/**
 * @brief 处理DeepSeek API响应
 * @param response: DeepSeek API的响应内容
 * @retval None
 */
void PetCare_Process_DeepSeek_Response(char* response)
{
    // 由于LCD屏幕有限，我们需要分行显示响应内容
    u16 y_pos = 30;
    u16 line_height = 16;
    u16 max_chars_per_line = 20; // 根据LCD宽度和字体大小调整
    u16 max_lines = 10; // 根据LCD高度和字体大小调整
    u16 response_len = strlen(response);
    u16 start_pos = 0;
    u16 line_count = 0;
    char line_buf[21] = {0}; // 多一个字节用于存放字符串结束符
    u16 chars_to_copy = 0;
    
    // 始终通过UART2发送DeepSeek响应内容到电脑
    u2_printf("DeepSeek Response: %s\r\n", response);
    
    // 显示DeepSeek响应内容
    LCD_Clear(WHITE);
    FRONT_COLOR = BLACK;
    LCD_ShowString(10, 10, 220, 16, 16, "DeepSeek Response:");
    
    while (start_pos < response_len && line_count < max_lines)
    {
        chars_to_copy = response_len - start_pos > max_chars_per_line ? max_chars_per_line : response_len - start_pos;
        
        memset(line_buf, 0, sizeof(line_buf));
        strncpy(line_buf, response + start_pos, chars_to_copy);
        LCD_ShowString(10, y_pos, 220, 16, 16, line_buf);
        
        start_pos += chars_to_copy;
        y_pos += line_height;
        line_count++;
    }
    
    // 如果响应内容太长，显示提示信息
    if (start_pos < response_len)
    {
        LCD_ShowString(10, y_pos, 220, 16, 16, "...(more content)");
    }
    
    // 显示返回提示
    LCD_ShowString(10, y_pos + line_height, 220, 16, 16, "Press KEY0 to return");
    
    // 等待用户按键返回
    while (KEY_Scan(0) != KEY0_PRESS);
    
    // 返回到之前的页面
    PetCare_Display_Page(pet_care_data.current_page);
}


/**
 * @brief 显示环境数据
 */
void PetCare_Display_Data(void)
{
    // 检查是否是由deepseek_bridge.py发送的命令触发的
    // 如果是，则不清屏，只更新必要的数据项
    if(strncmp((char*)USART3_RX_BUF, "+deepseek", 9) == 0 || 
       strncmp((char*)USART3_RX_BUF, "+deepseek_response", 18) == 0)
    {
        // 不执行任何操作，避免清屏
        return;
    }
    
    // 清除整个屏幕
    LCD_Clear(WHITE);
    
    // 显示当前页面
    PetCare_Display_Page(pet_care_data.current_page);
}

/**
 * @brief 更新特定数据项的显示，而不刷新整个屏幕
 * @param data_type: 要更新的数据类型
 */
void PetCare_Update_Display_Item(u8 data_type)
{
    char buf[50];
    
    // 根据当前页面和数据类型更新特定区�??
    switch(pet_care_data.current_page)
    {
        case PAGE_ENV_DEVICE: // 环境数据和设备状态页�??
            switch(data_type)
            {
                case 0: // 温度
                    FRONT_COLOR = BLUE;
                    sprintf(buf, "Temperature: %.1f C  ", pet_care_data.temperature);
                    // 先用背景色清除该区域
                    LCD_Fill(10, 60, 200, 76, WHITE);
                    // 显示新数�??
                    LCD_ShowString(10, 60, 200, 16, 16, (u8*)buf);
                    break;
                    
                case 1: // 湿度
                    FRONT_COLOR = BLUE;
                    sprintf(buf, "Humidity: %d%%  ", pet_care_data.humidity);
                    LCD_Fill(10, 90, 200, 106, WHITE);
                    LCD_ShowString(10, 90, 200, 16, 16, (u8*)buf);
                    break;
                    
                case 2: // 光照
                    FRONT_COLOR = BLUE;
                    sprintf(buf, "Light: %d%%  ", pet_care_data.light_value);
                    LCD_Fill(10, 120, 200, 136, WHITE);
                    LCD_ShowString(10, 120, 200, 16, 16, (u8*)buf);
                    break;
                    
                case 3: // 风扇状�?
                    FRONT_COLOR = BLUE;
                    sprintf(buf, "Fan: %s  ", pet_care_data.fan_status == DEVICE_ON ? "ON" : "OFF");
                    LCD_Fill(10, 150, 200, 166, WHITE);
                    LCD_ShowString(10, 150, 200, 16, 16, (u8*)buf);
                    break;
                    
                case 4: // 照明状�?
                    FRONT_COLOR = BLUE;
                    sprintf(buf, "Light: %s  ", pet_care_data.light_status == DEVICE_ON ? "ON" : "OFF");
                    LCD_Fill(10, 180, 200, 196, WHITE);
                    LCD_ShowString(10, 180, 200, 16, 16, (u8*)buf);
                    break;
                    
                case 5: // 蜂鸣器状�??
                    FRONT_COLOR = BLUE;
                    sprintf(buf, "Beep: %s  ", pet_care_data.beep_status == DEVICE_ON ? "ON" : "OFF");
                    LCD_Fill(10, 210, 200, 226, WHITE);
                    LCD_ShowString(10, 210, 200, 16, 16, (u8*)buf);
                    break;
                    
                case 6: // 自动控制状�?
                    FRONT_COLOR = BLUE;
                    sprintf(buf, "Auto: %s  ", pet_care_data.auto_control == DEVICE_ON ? "ON" : "OFF");
                    LCD_Fill(10, 240, 200, 256, WHITE);
                    LCD_ShowString(10, 240, 200, 16, 16, (u8*)buf);
                    break;
                    
                default:
                    break;
            }
            break;
            
        case PAGE_ROLE_STATUS: // 角色和状态页�??
            if(data_type == 7) // 系统状�?
            {
                // 清除系统状态区�??
                LCD_Fill(10, 60, 200, 76, WHITE);
                
                // 显示系统状�?
                switch(pet_care_data.system_status)
                {
                    case PET_CARE_NORMAL:
                        FRONT_COLOR = GREEN;
                        LCD_ShowString(10, 60, 200, 16, 16, "System Status: Normal  ");
                        break;
                    case PET_CARE_WARNING:
                        FRONT_COLOR = YELLOW;
                        LCD_ShowString(10, 60, 200, 16, 16, "System Status: Warning  ");
                        break;
                    case PET_CARE_ALARM:
                        FRONT_COLOR = RED;
                        LCD_ShowString(10, 60, 200, 16, 16, "System Status: Alarm!  ");
                        break;
                    default:
                        break;
                }
            }
            break;
            
        default:
            break;
    }
}

/**
 * @brief 切换显示页面
 */
void PetCare_Switch_Page(void)
{
    // 切换到下一�??
    pet_care_data.current_page = (pet_care_data.current_page + 1) % PAGE_MAX;
    
    // 显示新页�??
    PetCare_Display_Data();
}

/**
 * @brief 显示指定页面
 */
void PetCare_Display_Page(u8 page)
{
    char buf[50];
    
    // ��ʾ���⣨����ҳ�涼��ʾ��
    FRONT_COLOR = RED;
    LCD_ShowString(100, 10, tftlcd_data.width, tftlcd_data.height, 16, "Pet Care System");
    LCD_ShowString(100, 30, tftlcd_data.width, tftlcd_data.height, 16, "Smart Pet Guardian");
    
    // ����ҳ����ʾ��ͬ����
    switch(page)
    {
        case PAGE_ROLE_STATUS: // ϵͳ״̬���豸״̬ҳ��
            FRONT_COLOR = BLUE;
            // ��ʾϵͳ״̬
            switch(pet_care_data.system_status)
            {
                case PET_CARE_NORMAL:
                    FRONT_COLOR = GREEN;
                    LCD_ShowString(10, 60, 200, 16, 16, "System Status: Normal  ");
                    break;
                case PET_CARE_WARNING:
                    FRONT_COLOR = YELLOW;
                    LCD_ShowString(10, 60, 200, 16, 16, "System Status: Warning  ");
                    break;
                case PET_CARE_ALARM:
                    FRONT_COLOR = RED;
                    LCD_ShowString(10, 60, 200, 16, 16, "System Status: Alarm!  ");
                    break;
                default:
                    break;
            }
            
            // ��ʾ����״̬
            FRONT_COLOR = BLUE;
            if(HC05_Get_Role() == 1)
            {
                LCD_ShowString(10, 90, 200, 16, 16, "ROLE: Master");
            }
            else 
            {
                LCD_ShowString(10, 90, 200, 16, 16, "ROLE: Slave ");
            }
            
            if(HC05_LED)
            {
                LCD_ShowString(10, 120, 200, 16, 16, "STA: Connected ");
            }
            else 
            {
                LCD_ShowString(10, 120, 200, 16, 16, "STA: Disconnect");
            }

            // ��ʾ�豸״̬
            sprintf(buf, "Fan: %s  ", pet_care_data.fan_status == DEVICE_ON ? "ON" : "OFF");
            LCD_ShowString(10, 150, 200, 16, 16, (u8*)buf);
            
            sprintf(buf, "Light: %s  ", pet_care_data.light_status == DEVICE_ON ? "ON" : "OFF");
            LCD_ShowString(10, 180, 200, 16, 16, (u8*)buf);
            
            sprintf(buf, "Beep: %s  ", pet_care_data.beep_status == DEVICE_ON ? "ON" : "OFF");
            LCD_ShowString(10, 210, 200, 16, 16, (u8*)buf);
            
            sprintf(buf, "Auto: %s  ", pet_care_data.auto_control == DEVICE_ON ? "ON" : "OFF");
            LCD_ShowString(10, 240, 200, 16, 16, (u8*)buf);
            break;
            
        case PAGE_ENV_DEVICE: // ��������ҳ��
            FRONT_COLOR = BLUE;
            
            // ��ʾ�¶�
            sprintf(buf, "Temperature: %.1f C  ", pet_care_data.temperature);
            LCD_ShowString(10, 60, 200, 16, 16, (u8*)buf);
            
            // ��ʾʪ��
            sprintf(buf, "Humidity: %d%%  ", pet_care_data.humidity);
            LCD_ShowString(10, 90, 200, 16, 16, (u8*)buf);
            
            // ��ʾ����
            sprintf(buf, "Light: %d%%  ", pet_care_data.light_value);
            LCD_ShowString(10, 120, 200, 16, 16, (u8*)buf);
            break;
            
        case PAGE_PICTURE: // ͼƬҳ�汣�ֲ���
            FRONT_COLOR = BLUE;
            LCD_ShowString(10, 60, 200, 16, 16, "Pet Picture:");
            
            // 添加调试信息
            printf("Display Page - Light: %d, Temperature: %.1f\r\n", pet_care_data.light_value, pet_care_data.temperature);
            
            // 清除之前的图片区�??
            BACK_COLOR = WHITE;
            LCD_Fill(10, 100, tftlcd_data.width-10, 250, WHITE);
            
            // 根据温度和光照条件显示不同的图片
            if(pet_care_data.light_value > 80) {
                // 光照高于40时显示unhappy的图�??
                printf("Showing unhappy picture due to high light (%d)\r\n", pet_care_data.light_value);
                LCD_ShowString(10, 80, 200, 16, 16, "Pet is unhappy due to high light!");
                LCD_ShowPicture((tftlcd_data.width - 200) / 2, 100, 200, 151, (u8*)gImage_doro_unhappy);
            } else if(pet_care_data.temperature >= 28.0) {
                // 温度高于28度时显示hot的图�??
                printf("Showing hot picture due to high temperature (%.1f)\r\n", pet_care_data.temperature);
                LCD_ShowString(10, 80, 300, 16, 16, "Pet is hot due to high temperature!");
                LCD_ShowPicture((tftlcd_data.width - 96) / 2, 100, 96, 100, (u8*)gImage_doro_hot);
            } else {
                // 温度和光照都正常时显示普通图�??
                printf("Showing normal picture (Temp: %.1f, Light: %d)\r\n", pet_care_data.temperature, pet_care_data.light_value);
                LCD_ShowString(10, 80, 200, 16, 16, "Pet is comfortable!");
                LCD_ShowPicture((tftlcd_data.width - 100) / 2, 100, 100, 91, (u8*)gImage_doro2);
            }
            break;
            
        default:
            break;
    }
}

/**
 * @brief 设置风扇状�?
 * @param status: DEVICE_ON/DEVICE_OFF
 */
void PetCare_Set_Fan(u8 status)
{
    // 检查状态是否变�??
    u8 old_status = pet_care_data.fan_status;
    
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
    
    // 如果状态变化，则更新风扇状态显�??
    if(old_status != pet_care_data.fan_status)
    {
        // 只有在环境数据和设备状态页面才需要更新显�??
        if(pet_care_data.current_page == PAGE_ENV_DEVICE)
        {
            PetCare_Update_Display_Item(3); // 更新风扇状态显�??
        }
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
        // 开启白色照�????
        u8 i, j;
        for(i=0; i<RGB_LED_YHIGH; i++)
        {
            for(j=0; j<RGB_LED_XWIDTH; j++)
            {
                RGB_LED_Write_24Bits(255, 255, 255); // 白色 (RGB全亮)
            }
        }
        pet_care_data.light_status = DEVICE_ON;
    }
    else
    {
        RGB_LED_Clear(); // 关闭照明
        pet_care_data.light_status = DEVICE_OFF;
    }
}*/

void PetCare_Set_Light(u8 status)
{
    // 检查状态是否变�??
    u8 old_status = pet_care_data.light_status;
    
    if(status == DEVICE_ON)
    {
        u32 color;
        if(pet_care_data.light_value < 20)
            color = RGB_COLOR_BLUE;
        else if(pet_care_data.light_value < 30)
            color = RGB_COLOR_PINK;
        else
            color = RGB_COLOR_GREEN;
				
        /*for(i = 0; i < RGB_LED_YHIGH; i++)
        {
            for(j = 0; j < RGB_LED_XWIDTH; j++)
            {
                
                RGB_DrawDotColor(j, i, 1, color); 
            }
        }	*/	
        RGB_DrawHeart(color);

        pet_care_data.light_status = DEVICE_ON;
    }
    else
    {
        RGB_LED_Clear(); // 关闭照明
        pet_care_data.light_status = DEVICE_OFF;
    }
    
    // 如果状态变化，则更新腧明状态显�??
    if(old_status != pet_care_data.light_status)
    {
        // 只有在环境数据和设备状态页面才需要更新显�??
        if(pet_care_data.current_page == PAGE_ENV_DEVICE)
        {
            PetCare_Update_Display_Item(4); // 更新照明状态显�??
        }
    }
}







/**
 * @brief 设置蜂鸣器状�????
 * @param status: DEVICE_ON/DEVICE_OFF
 */
void PetCare_Set_Beep(u8 status)
{
    // 检查状态是否变�??
    u8 old_status = pet_care_data.beep_status;
    
    if(status == DEVICE_ON)
    {
        BEEP = 1; // 开启蜂鸣器
        pet_care_data.beep_status = DEVICE_ON;
    }
    else
    {
        BEEP = 0; // 关闭蜂鸣�??
        pet_care_data.beep_status = DEVICE_OFF;
    }
    
    // 如果状态变化，则更新蜂鸣器状态显�??
    if(old_status != pet_care_data.beep_status)
    {
        // 只有在环境数据和设备状态页面才需要更新显�??
        if(pet_care_data.current_page == PAGE_ENV_DEVICE)
        {
            PetCare_Update_Display_Item(5); // 更新蜂鸣器状态显�??
        }
    }
}

/**
 * @brief 设置自动控制状�?
 * @param status: DEVICE_ON/DEVICE_OFF
 */
void PetCare_Set_Auto_Control(u8 status)
{
    // 检查状态是否变�??
    u8 old_status = pet_care_data.auto_control;
    
    pet_care_data.auto_control = status;
    
    if(status != DEVICE_ON)
    {
        // 关闭自动控制时，默认关闭所有设�??
        PetCare_Set_Fan(DEVICE_OFF);
        PetCare_Set_Light(DEVICE_OFF);
        PetCare_Set_Beep(DEVICE_OFF);
    }
    else if(old_status != status)
    {
        // 如果是从关闭状态切换到开启状态，则更新自动控制状态显�??
        // 只有在环境数据和设备状态页面才需要更新显�??
        if(pet_care_data.current_page == PAGE_ENV_DEVICE)
        {
            PetCare_Update_Display_Item(6); // 更新自动控制状态显�??
        }
    }
}

/**
 * @brief 处理命令（支持UART2和UART3）
 * @param cmd: 命令字符串
 */
void PetCare_Process_Command(char* cmd)
{
    PetCare_Process_Command_Ex(cmd, 0); // 默认使用UART3（蓝牙）
}

/**
 * @brief 处理命令（扩展版本，支持指定输出端口）
 * @param cmd: 命令字符串
 * @param output_port: 0=UART3(蓝牙), 1=UART2(电脑)
 */
void PetCare_Process_Command_Ex(char* cmd, u8 output_port)
{
    printf("Received command: %s\r\n", cmd);
    
    // 风扇控制命令
    if(strcmp(cmd, "+FAN ON\r\n") == 0)
    {
        PetCare_Set_Auto_Control(DEVICE_OFF); // 关闭自动控制
        PetCare_Set_Fan(DEVICE_ON);
        printf("Fan turned ON\r\n");
        if(output_port == 1) u2_printf("Fan turned ON\r\n");
        else u3_printf("Fan turned ON\r\n");
    }
    else if(strcmp(cmd, "+FAN OFF\r\n") == 0)
    {
        PetCare_Set_Auto_Control(DEVICE_OFF); // 关闭自动控制
        PetCare_Set_Fan(DEVICE_OFF);
        printf("Fan turned OFF\r\n");
        if(output_port == 1) u2_printf("Fan turned OFF\r\n");
        else u3_printf("Fan turned OFF\r\n");
    }
    
    // 照明控制命令
    else if(strcmp(cmd, "+LIGHT ON\r\n") == 0)
    {
        PetCare_Set_Auto_Control(DEVICE_OFF); // 关闭自动控制
        PetCare_Set_Light(DEVICE_ON);
        printf("Light turned ON\r\n");
        if(output_port == 1) u2_printf("Light turned ON\r\n");
        else u3_printf("Light turned ON\r\n");
    }
    else if(strcmp(cmd, "+LIGHT OFF\r\n") == 0)
    {
        PetCare_Set_Auto_Control(DEVICE_OFF); // 关闭自动控制
        PetCare_Set_Light(DEVICE_OFF);
        printf("Light turned OFF\r\n");
        if(output_port == 1) u2_printf("Light turned OFF\r\n");
        else u3_printf("Light turned OFF\r\n");
    }
    
    // 蜂鸣器控制命�????
    else if(strcmp(cmd, "+BEEP ON\r\n") == 0)
    {
        PetCare_Set_Auto_Control(DEVICE_OFF); // 关闭自动控制
        PetCare_Set_Beep(DEVICE_ON);
        printf("Beep turned ON\r\n");
        if(output_port == 1) u2_printf("Beep turned ON\r\n");
        else u3_printf("Beep turned ON\r\n");
    }
    else if(strcmp(cmd, "+BEEP OFF\r\n") == 0)
    {
        PetCare_Set_Auto_Control(DEVICE_OFF); // 关闭自动控制
        PetCare_Set_Beep(DEVICE_OFF);
        printf("Beep turned OFF\r\n");
        if(output_port == 1) u2_printf("Beep turned OFF\r\n");
        else u3_printf("Beep turned OFF\r\n");
    }
    
    // 自动控制命令
    else if(strcmp(cmd, "+AUTO ON\r\n") == 0)
    {
        PetCare_Set_Auto_Control(DEVICE_ON);
        printf("Auto control turned ON\r\n");
        if(output_port == 1) u2_printf("Auto control turned ON\r\n");
        else u3_printf("Auto control turned ON\r\n");
    }
    else if(strcmp(cmd, "+AUTO OFF\r\n") == 0)
    {
        PetCare_Set_Auto_Control(DEVICE_OFF);
        printf("Auto control turned OFF\r\n");
        if(output_port == 1) u2_printf("Auto control turned OFF\r\n");
        else u3_printf("Auto control turned OFF\r\n");
    }
    
    // 查询状态命�????
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
        
        if(output_port == 1) {
            u2_printf("Temperature: %.1f C\r\n", pet_care_data.temperature);
            u2_printf("Humidity: %d%%\r\n", pet_care_data.humidity);
            u2_printf("Light: %d%%\r\n", pet_care_data.light_value);
            u2_printf("System Status: %d\r\n", pet_care_data.system_status);
            u2_printf("Fan: %s\r\n", pet_care_data.fan_status == DEVICE_ON ? "ON" : "OFF");
            u2_printf("Light: %s\r\n", pet_care_data.light_status == DEVICE_ON ? "ON" : "OFF");
            u2_printf("Beep: %s\r\n", pet_care_data.beep_status == DEVICE_ON ? "ON" : "OFF");
            u2_printf("Auto Control: %s\r\n", pet_care_data.auto_control == DEVICE_ON ? "ON" : "OFF");
        } else {
            u3_printf("Temperature: %.1f C\r\n", pet_care_data.temperature);
            u3_printf("Humidity: %d%%\r\n", pet_care_data.humidity);
            u3_printf("Light: %d%%\r\n", pet_care_data.light_value);
            u3_printf("System Status: %d\r\n", pet_care_data.system_status);
            u3_printf("Fan: %s\r\n", pet_care_data.fan_status == DEVICE_ON ? "ON" : "OFF");
            u3_printf("Light: %s\r\n", pet_care_data.light_status == DEVICE_ON ? "ON" : "OFF");
            u3_printf("Beep: %s\r\n", pet_care_data.beep_status == DEVICE_ON ? "ON" : "OFF");
            u3_printf("Auto Control: %s\r\n", pet_care_data.auto_control == DEVICE_ON ? "ON" : "OFF");
        }
    }else if(strcmp(cmd,"+page turn left\r\n") == 0)
    {
        pet_care_data.current_page  = (pet_care_data.current_page+1)%3;
    }else if(strcmp(cmd,"+page turn right\r\n") == 0)
    {
        pet_care_data.current_page = (pet_care_data.current_page+3-1)%3;
    }

    if(strcmp(cmd,"+music on\r\n")==0)
    {
        printf("music play\r\n");
        if(output_port == 1) u2_printf("music play\r\n");
        else u3_printf("music play\r\n");
        music_sample();
    }else if(strncmp(cmd,"+deepseek ", 10) == 0)
    {
        // 提取DeepSeek查询内容
        char query[200] = {0};
        strncpy(query, cmd + 10, strlen(cmd) - 12); // 去掉命令前缀和末尾的\r\n
        // 通过蓝牙发送确认消息
        printf("Processing DeepSeek query: %s\r\n", query);
        
        // DeepSeek查询始终使用UART2发送到电脑，无论output_port参数如何
        u2_printf("DeepSeek Query: %s\r\n", query);
        
        // 不在LCD上显示查询内容
        // LCD_Clear(WHITE);
        // FRONT_COLOR = BLACK;
        // LCD_ShowString(10, 10, 220, 16, 16, "DeepSeek Query:");
        // LCD_ShowString(10, 30, 220, 16, 16, query);
        // LCD_ShowString(10, 50, 220, 16, 16, "Waiting for response...");
    }
    else if(strncmp(cmd,"+deepseek_response ", 19) == 0)
    {
        // 提取DeepSeek响应内容
        char response[200] = {0};
        strncpy(response, cmd + 19, strlen(cmd) - 21); // 去掉命令前缀和末尾的\r\n
        // DeepSeek响应始终使用UART2处理，无论output_port参数如何
        u3_printf("DeepSeek Response: %s\r\n", response);
        
        // 不调用PetCare_Process_DeepSeek_Response函数，避免在LCD上显示
        // PetCare_Process_DeepSeek_Response(response);
    }
}