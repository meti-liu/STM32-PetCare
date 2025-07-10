# PetCare STM32

## 项目简介

PetCare STM32是一个基于STM32F103ZE的宠物照护系统，通过各种传感器监测环境参数（温度、湿度、光照等），并通过风扇、照明等设备进行环境控制，为宠物提供舒适的生活环境。

## 功能特点

- 实时监测环境温度、湿度和光照
- 自动控制风扇、照明和蜂鸣器
- 通过LCD显示环境数据和系统状态
- 支持按键控制和蓝牙远程控制
- 支持通过蓝牙调用DeepSeek API进行智能问答

## 硬件组成

- STM32F103ZE微控制器
- DHT11温湿度传感器
- 光敏电阻（光照传感器）
- HC05蓝牙模块
- LCD显示屏
- 风扇（PWM控制）
- RGB彩灯（WS2812）
- 蜂鸣器
- 按键

## 软件架构

- APP：应用层代码，包含各模块的驱动和应用逻辑
- Libraries：STM32标准外设库
- Public：公共功能模块（SysTick、USART等）
- User：用户代码（main函数、中断处理等）

## DeepSeek API集成

本项目支持通过蓝牙调用DeepSeek API进行智能问答。详细说明请参考[DeepSeek API集成说明](./DeepSeek_API_README.md)。

### 实现方式

1. STM32通过HC05蓝牙模块发送查询请求
2. 电脑/手机接收查询请求，调用DeepSeek API
3. 电脑/手机将API响应通过蓝牙发送回STM32
4. STM32接收并显示响应内容

### 辅助工具

- [Python桥接脚本](./deepseek_bridge.py)：用于在电脑端接收查询请求并调用API
- [Android应用](./Android_Example/)：用于在手机端接收查询请求并调用API

## 使用方法

### 基本操作

1. 编译并烧录代码到STM32开发板
2. 连接所有传感器和设备
3. 上电启动系统
4. 使用按键或蓝牙命令控制系统

### DeepSeek API查询

通过蓝牙发送以下格式的命令：

```
+deepseek 你的问题内容
```

例如：`+deepseek 今天天气怎么样？`

## 蓝牙命令列表

- `+fan on` - 开启风扇
- `+fan off` - 关闭风扇
- `+light on` - 开启照明
- `+light off` - 关闭照明
- `+beep on` - 开启蜂鸣器
- `+beep off` - 关闭蜂鸣器
- `+auto on` - 开启自动控制
- `+auto off` - 关闭自动控制
- `+status` - 查询系统状态
- `+page turn left` - 向左翻页
- `+page turn right` - 向右翻页
- `+music on` - 播放音乐
- `+deepseek 问题内容` - 发送DeepSeek API查询

## 开发环境

- Keil MDK 5.x
- STM32F10x标准外设库