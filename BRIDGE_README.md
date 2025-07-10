# STM32-PetCare DeepSeek桥接工具

## 项目概述

本项目提供了一个Python桥接脚本，用于连接STM32-PetCare系统与DeepSeek API。通过这个桥接工具，STM32设备可以通过蓝牙发送查询请求，桥接脚本接收请求并调用DeepSeek API，然后将响应发送回STM32设备。

## 文件说明

- `deepseek_bridge.py`：主要的桥接脚本
- `deepseek_bridge_usage.md`：详细的使用说明文档
- `start_bridge.bat`：Windows一键启动脚本

## 快速开始

### 环境要求

- Python 3.6+
- pyserial库
- requests库

### 安装依赖

```bash
pip install pyserial requests
```

### 配置

在`deepseek_bridge.py`文件中，修改以下配置参数：

```python
# 配置参数
SERIAL_PORT = 'COM3'  # 根据实际蓝牙串口修改
BAUDRATE = 9600
DEEPSEEK_API_KEY = 'sk-94432043ca50474083056112bcfe26b5'  # 替换为你的DeepSeek API密钥
```

### 运行

Windows用户可以直接双击`start_bridge.bat`文件启动桥接脚本。

或者，在命令行中运行：

```bash
python deepseek_bridge.py
```

## 使用流程

1. 启动桥接脚本
2. 确保STM32设备已连接HC05蓝牙模块
3. 确保电脑已与HC05蓝牙模块配对成功
4. 在STM32设备上发送查询命令：`+deepseek 你的问题内容`
5. 桥接脚本接收请求，调用DeepSeek API，并将响应发送回STM32
6. STM32接收响应并在LCD上显示

## 故障排除

如果遇到问题，请参考`deepseek_bridge_usage.md`文件中的故障排除部分。

## 改进日志

### 2024-07-07

- 增强错误处理和日志输出
- 添加详细的使用说明文档
- 创建Windows一键启动脚本
- 验证Python环境和依赖库

## 注意事项

- 确保DeepSeek API密钥有效且有足够的调用额度
- 蓝牙通信可能存在延迟，请耐心等待响应
- 如果响应内容过长，可能需要分段发送，当前实现可能需要根据实际情况调整