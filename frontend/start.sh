#!/bin/bash
# ========================================
#   火车票管理系统 - Flask 前端启动脚本 (Linux)
# ========================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "========================================"
echo "  火车票管理系统 - Flask 前端"
echo "========================================"
echo ""

# 检查 Python
if ! command -v python3 &>/dev/null; then
    echo "[错误] 未找到 python3，请先安装 Python 3.9+"
    exit 1
fi

# 检查 C++ 后端
if [ ! -f "../code" ]; then
    echo "[警告] 未找到 C++ 后端可执行文件: ../code"
    echo "请先编译 C++ 项目："
    echo "  cd .."
    echo "  cmake -B build"
    echo "  cmake --build build"
    echo ""
fi

# 安装依赖
echo "[1/2] 安装 Python 依赖..."
pip3 install flask -q 2>/dev/null || pip install flask -q 2>/dev/null || {
    echo "[错误] pip 安装失败，请手动执行: pip install flask"
    exit 1
}

# 启动
echo "[2/2] 启动 Flask 服务器..."
echo ""
echo "  浏览器访问: http://127.0.0.1:5000"
echo "  按 Ctrl+C 停止服务器"
echo "========================================"
echo ""

python3 app.py || python app.py
