@echo off
chcp 65001 >nul
echo ========================================
echo   火车票管理系统 - Flask 前端启动脚本
echo ========================================
echo.

cd /d "%~dp0"

:: 检查 Python 是否安装
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [错误] 未找到 Python，请先安装 Python 3.9+
    pause
    exit /b 1
)

:: 检查 C++ 后端是否编译
if not exist "..\code" (
    if not exist "..\code.exe" (
        echo [警告] 未找到 C++ 后端可执行文件 (code / code.exe)
        echo 请先编译 C++ 项目：
        echo   cd ..
        echo   cmake -B build
        echo   cmake --build build
        echo.
    )
)

:: 安装依赖
echo [1/2] 安装 Python 依赖...
pip install flask -q
if %errorlevel% neq 0 (
    echo [错误] pip 安装失败，请检查网络连接
    pause
    exit /b 1
)

:: 启动 Flask
echo [2/2] 启动 Flask 服务器...
echo.
echo 请在浏览器中打开: http://127.0.0.1:5000
echo 按 Ctrl+C 停止服务器
echo ========================================
echo.

python app.py

pause
