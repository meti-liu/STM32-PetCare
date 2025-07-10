# STM32-PetCare DeepSeek API 集成说明

本文档说明如何通过蓝牙将STM32-PetCare系统与DeepSeek API集成，实现智能问答功能。

## 系统架构

系统采用以下架构实现DeepSeek API的调用：

1. STM32通过HC05蓝牙模块发送查询请求
2. 电脑/手机接收查询请求，调用DeepSeek API
3. 电脑/手机将API响应通过蓝牙发送回STM32
4. STM32接收并显示响应内容

## 使用方法

### STM32端

1. 编译并烧录更新后的固件到STM32开发板
2. 确保HC05蓝牙模块正确连接并初始化
3. 通过蓝牙向STM32发送以下格式的命令来查询DeepSeek API：
   ```
   +deepseek 你的问题内容
   ```
   例如：`+deepseek 今天天气怎么样？`

### 电脑端

1. 安装必要的Python库：
   ```
   pip install pyserial requests
   ```

2. 修改`deepseek_bridge.py`脚本中的配置参数：
   - `SERIAL_PORT`：设置为HC05蓝牙模块对应的串口（例如Windows上的COM3）
   - `DEEPSEEK_API_KEY`：设置为你的DeepSeek API密钥

3. 运行桥接脚本：
   ```
   python deepseek_bridge.py
   ```

4. 脚本将监听来自STM32的查询请求，调用DeepSeek API，并将响应发送回STM32

## 命令格式

### 查询请求（STM32 -> 电脑）

```
DeepSeek Query: 你的问题内容
```

### 响应（电脑 -> STM32）

```
+deepseek_response API响应内容
```

## 注意事项

1. 确保DeepSeek API密钥有效且有足够的调用额度
2. 由于LCD屏幕大小限制，长响应内容会被截断显示
3. 按KEY0键可以从响应显示页面返回到主界面
4. 蓝牙通信可能存在延迟，请耐心等待响应
5. 如果响应内容过长，可能需要分段发送，当前实现可能需要根据实际情况调整

## 故障排除

1. 确保蓝牙连接正常，可以通过发送其他命令（如`+status`）测试
2. 检查串口配置是否正确（波特率、端口号等）
3. 检查DeepSeek API密钥是否正确
4. 查看Python脚本的控制台输出，了解可能的错误信息