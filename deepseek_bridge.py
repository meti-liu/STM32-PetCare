import serial
import time
import re
import requests
import json

# 串口配置
SERIAL_PORT = 'COM4'  # UART2端口，用于与STM32的电脑通信（请根据实际COM端口修改）
BAUDRATE = 9600       # 波特率，与STM32的UART2配置匹配
DEEPSEEK_API_KEY = 'sk-94432043ca50474083056112bcfe26b5'  # 替换为你的DeepSeek API密钥

# 初始化串口
def init_serial():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)
        print(f"成功连接到串口 {SERIAL_PORT}")
        return ser
    except Exception as e:
        print(f"连接串口失败: {e}")
        return None

# 调用DeepSeek API
def call_deepseek_api(query):
    url = "https://api.deepseek.com/chat/completions"
    headers = {
        "Content-Type": "application/json",
        "Authorization": f"Bearer {DEEPSEEK_API_KEY}"
    }
    data = {
        "model": "deepseek-chat",
        "messages": [
            {"role": "system", "content": "你是一个有用的助手，不要用markdown回答我，回答控制在100字内，不要使用奇奇怪怪的符号"},
            {"role": "user", "content": query}
        ],
        "stream": False
    }
    
    try:
        print(f"正在调用DeepSeek API，查询内容: {query}")
        response = requests.post(url, headers=headers, json=data)
        if response.status_code == 200:
            result = response.json()
            return result['choices'][0]['message']['content']
        else:
            print(f"API调用失败: {response.status_code} - {response.text}")
            return f"API调用失败: {response.status_code} - {response.text}"
    except Exception as e:
        print(f"API调用异常: {e}")
        return f"API调用异常: {e}"

# 主循环
def main():
    ser = init_serial()
    if not ser:
        print("串口初始化失败，请检查串口配置或蓝牙连接")
        return
    
    # 不发送任何初始化命令，避免触发LCD清屏
    print("等待STM32发送DeepSeek查询请求...")
    print(f"当前串口配置: 端口={SERIAL_PORT}, 波特率={BAUDRATE}")
    
    try:
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                print(f"收到: {line}")
                
                # 检查是否是DeepSeek查询请求
                match = re.search(r'DeepSeek Query: (.+)', line)
                if match:
                    query = match.group(1)
                    print(f"处理查询: {query}")
                    
                    # 调用DeepSeek API
                    response = call_deepseek_api(query)
                    print(f"API响应: {response}")
                    
                    # 将响应发送回STM32
                    # 由于响应可能很长，我们可能需要分段发送
                    # 这里我们简化处理，假设响应不会太长
                    cmd = f"+deepseek_response {response}\r\n"
                    ser.write(cmd.encode('utf-8'))
                    print(f"已发送响应到STM32")
            
            time.sleep(0.1)  # 短暂休眠，避免CPU占用过高
    
    except KeyboardInterrupt:
        print("程序已终止")
    except Exception as e:
        print(f"发生错误: {e}")
    finally:
        if ser and ser.is_open:
            ser.close()
            print("串口已关闭")

if __name__ == "__main__":
    main()