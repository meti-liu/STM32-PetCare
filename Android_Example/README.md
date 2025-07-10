# DeepSeek蓝牙桥接Android应用

这个Android应用用于连接STM32-PetCare系统的HC05蓝牙模块，接收查询请求，调用DeepSeek API，并将响应发送回STM32设备。

## 功能特点

- 通过蓝牙连接HC05模块
- 接收STM32发送的DeepSeek查询请求
- 调用DeepSeek API获取响应
- 将响应发送回STM32设备
- 支持手动输入查询内容

## 使用前准备

1. 确保HC05蓝牙模块已正确配置并与STM32连接
2. 获取HC05蓝牙模块的MAC地址
3. 申请DeepSeek API密钥

## 配置应用

在`MainActivity.java`文件中，修改以下配置参数：

```java
private String deviceAddress = "00:00:00:00:00:00"; // 替换为HC05的MAC地址
private static final String DEEPSEEK_API_KEY = "your_api_key_here"; // 替换为你的DeepSeek API密钥
```

## 使用方法

1. 安装应用到Android设备
2. 确保Android设备的蓝牙已开启
3. 启动应用
4. 点击"连接蓝牙设备"按钮连接到HC05模块
5. 连接成功后，可以通过以下两种方式使用：
   - 手动输入查询内容并点击"发送查询"按钮
   - 等待STM32发送查询请求，应用会自动处理并返回响应

## 通信协议

### 从STM32接收查询请求

格式：`DeepSeek Query: 查询内容`

### 向STM32发送响应

格式：`+deepseek_response 响应内容`

## 权限要求

应用需要以下权限：

- BLUETOOTH：用于基本蓝牙功能
- BLUETOOTH_ADMIN：用于启用蓝牙
- BLUETOOTH_CONNECT：用于Android 12及以上版本的蓝牙连接

## 构建应用

1. 在Android Studio中创建新项目
2. 复制`MainActivity.java`到项目的主活动类
3. 复制`activity_main.xml`到项目的布局文件夹
4. 在`AndroidManifest.xml`中添加必要的蓝牙权限
5. 构建并安装应用

## 注意事项

- 确保DeepSeek API密钥有效且有足够的调用额度
- 蓝牙通信可能存在延迟，请耐心等待响应
- 如果响应内容过长，可能需要分段发送，当前实现可能需要根据实际情况调整
- 应用需要网络连接以调用DeepSeek API