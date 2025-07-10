@echo off
echo 正在启动DeepSeek桥接脚本...
echo.
echo 如果Python未安装或未添加到PATH，请先安装Python并确保添加到PATH
echo 如果依赖库未安装，请先运行: pip install pyserial requests
echo.
python deepseek_bridge.py
pause