#ifndef _pet_care_H
#define _pet_care_H

#include "system.h"
#include "dht11.h"
#include "lsens.h"
#include "beep.h"
#include "pwm.h"
#include "ws2812.h"

// 系统状态定义
#define PET_CARE_NORMAL      0   // 正常状态
#define PET_CARE_WARNING     1   // 警告状态
#define PET_CARE_ALARM       2   // 报警状态

// 环境参数阈值定义
#define TEMP_WARNING_HIGH    28.0  // 温度警告上限
#define TEMP_ALARM_HIGH      32.0  // 温度报警上限
#define TEMP_WARNING_LOW     15.0  // 温度警告下限
#define TEMP_ALARM_LOW       10.0  // 温度报警下限

#define LIGHT_WARNING_LOW    50    // 光照警告下限
#define LIGHT_ALARM_LOW      15    // 光照报警下限

// 设备控制状态定义
#define DEVICE_OFF           0     // 设备关闭
#define DEVICE_ON            1     // 设备开启
#define DEVICE_AUTO          2     // 设备自动控制

// 系统参数结构体
typedef struct
{
    float temperature;      // 当前温度
    u8 humidity;            // 当前湿度
    u8 light_value;         // 当前光照值
    u8 system_status;       // 系统状态
    u8 fan_status;          // 风扇状态
    u8 light_status;        // 照明状态
    u8 beep_status;         // 蜂鸣器状态
    u8 auto_control;        // 自动控制开关
} PetCare_TypeDef;

// 函数声明
void PetCare_Init(void);                    // 初始化宠物照护系统
void PetCare_Update_Data(void);             // 更新环境数据
void PetCare_Auto_Control(void);            // 自动控制设备
void PetCare_Display_Data(void);            // 显示环境数据
void PetCare_Set_Fan(u8 status);            // 设置风扇状态
void PetCare_Set_Light(u8 status);          // 设置照明状态
void PetCare_Set_Beep(u8 status);           // 设置蜂鸣器状态
void PetCare_Set_Auto_Control(u8 status);   // 设置自动控制状态
void PetCare_Process_Command(char* cmd);    // 处理蓝牙命令

// 全局变量声明
extern PetCare_TypeDef pet_care_data;

#endif
