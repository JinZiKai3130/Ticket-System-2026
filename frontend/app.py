"""
火车票管理系统 - Flask 前端
Flask 后端：通过子进程与 C++ 后端交互（stdin/stdout）
"""

from flask import Flask, render_template, request, jsonify
import subprocess
import threading
import os
import time
import re
import select
import signal
import sys

app = Flask(__name__)

# ============ C++ 后端子进程管理 ============

class BackendProcess:
    """管理与 C++ 后端的 stdin/stdout 通信"""
    
    def __init__(self, exe_path):
        self.exe_path = exe_path
        self.process = None
        self.lock = threading.Lock()
        self.timestamp = 0
        
    def start(self):
        """启动 C++ 后端进程"""
        if self.process is not None:
            return
        self.process = subprocess.Popen(
            [self.exe_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            cwd=os.path.dirname(self.exe_path)
        )
        self.timestamp = 0
        print(f"[Backend] C++ 后端已启动: {self.exe_path}")
        
    def stop(self):
        """停止 C++ 后端进程"""
        if self.process is None:
            return
        try:
            self.process.stdin.write(f"[{self.timestamp}] exit\n".encode('utf-8'))
            self.process.stdin.flush()
            self.process.wait(timeout=3)
        except:
            self.process.kill()
        self.process = None
        print("[Backend] C++ 后端已关闭")

    def _read_response(self, multi_line: bool) -> str:
        """统一的响应读取方法（Linux/Windows 兼容）"""
        fd = self.process.stdout.fileno()
        all_output = []

        while True:
            ready, _, _ = select.select([fd], [], [], 0.3)
            if not ready:
                break
            data = os.read(fd, 8192)
            if not data:
                break
            all_output.append(data.decode('utf-8', errors='replace'))
            if not multi_line:
                # 单行模式：读到换行就停
                if '\n' in all_output[-1]:
                    break

        result = "".join(all_output).strip()
        return result

    def send_command(self, command: str) -> str:
        """发送命令到 C++ 后端，返回单行输出"""
        with self.lock:
            if self.process is None:
                return "[-1] 后端未启动"
            self.timestamp += 1
            full_cmd = f"[{self.timestamp}] {command}\n"
            print(f"[Backend] 发送: {full_cmd.strip()}")
            try:
                self.process.stdin.write(full_cmd.encode('utf-8'))
                self.process.stdin.flush()
                result = self._read_response(multi_line=False)
                print(f"[Backend] 收到: {result[:100]}")
                # 只取第一行
                return result.split('\n')[0] if result else ""
            except Exception as e:
                return f"[-1] 通信错误: {str(e)}"

    def send_command_multi(self, command: str) -> str:
        """发送命令并读取多行输出"""
        with self.lock:
            if self.process is None:
                return "[-1] 后端未启动"
            self.timestamp += 1
            full_cmd = f"[{self.timestamp}] {command}\n"
            print(f"[Backend] 发送: {full_cmd.strip()}")
            try:
                self.process.stdin.write(full_cmd.encode('utf-8'))
                self.process.stdin.flush()
                result = self._read_response(multi_line=True)
                lines_count = result.count('\n') + 1 if result else 0
                print(f"[Backend] 收到 {lines_count} 行")
                return result
            except Exception as e:
                return f"[-1] 通信错误: {str(e)}"


# ============ 初始化后端 ============

# 可执行文件路径（相对于项目根目录）
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXE_PATH = os.path.join(PROJECT_ROOT, "code")

backend = BackendProcess(EXE_PATH)

# 应用启动时自动启动后端
try:
    if os.path.exists(EXE_PATH):
        backend.start()
        print(f"[Init] 找到可执行文件: {EXE_PATH}")
    else:
        print(f"[Init] 警告: 未找到可执行文件 {EXE_PATH}，请先编译 C++ 项目")
except Exception as e:
    print(f"[Init] 启动后端失败: {e}")


# ============ API 路由 ============

def remove_timestamp(output: str) -> str:
    """去除输出中的 [timestamp] 前缀"""
    return re.sub(r'^\[\d+\]\s*', '', output)


@app.route('/')
def index():
    """主页面"""
    return render_template('index.html')


@app.route('/api/status')
def api_status():
    """检查后端状态"""
    if backend.process is not None and backend.process.poll() is None:
        return jsonify({"status": "running", "message": "C++ 后端运行中"})
    return jsonify({"status": "stopped", "message": "C++ 后端未运行"})


@app.route('/api/restart')
def api_restart():
    """重启后端"""
    backend.stop()
    time.sleep(0.5)
    backend.start()
    return jsonify({"result": "后端已重启"})


@app.route('/api/command', methods=['POST'])
def api_command():
    """通用命令接口"""
    data = request.get_json()
    command = data.get('command', '')
    if not command:
        return jsonify({"result": "[-1] 空命令"})
    
    output = backend.send_command(command)
    return jsonify({"result": remove_timestamp(output)})


@app.route('/api/command_multi', methods=['POST'])
def api_command_multi():
    """多行命令接口（用于 query_train, query_ticket, query_order 等）"""
    data = request.get_json()
    command = data.get('command', '')
    if not command:
        return jsonify({"result": "[-1] 空命令"})
    
    output = backend.send_command_multi(command)
    # 处理多行输出的时间戳
    lines = output.split('\n')
    if lines:
        lines[0] = remove_timestamp(lines[0])
    return jsonify({"result": "\n".join(lines)})


# ============ 各功能 API ============

@app.route('/api/add_user', methods=['POST'])
def api_add_user():
    data = request.json
    
    # 首个用户注册时前端会传 first_user=true，且省略 c/g 字段
    # 此时用占位符填充 -c 和 -g（后端 C++ 会自动忽略）
    if data.get('first_user'):
        c = '_'
        g = '10'
    else:
        c = data['c']
        g = data['g']
    
    cmd = (f"add_user -c {c} -u {data['u']} "
           f"-p {data['p']} -n {data['n']} "
           f"-m {data['m']} -g {g}")
    output = backend.send_command(cmd)
    return jsonify({"result": remove_timestamp(output)})


@app.route('/api/login', methods=['POST'])
def login():
    """登录: -u -p"""
    data = request.get_json()
    cmd = f"login -u {data['u']} -p {data['p']}"
    output = backend.send_command(cmd)
    return jsonify({"result": remove_timestamp(output)})


@app.route('/api/logout', methods=['POST'])
def logout():
    """登出: -u"""
    data = request.get_json()
    cmd = f"logout -u {data['u']}"
    output = backend.send_command(cmd)
    return jsonify({"result": remove_timestamp(output)})


@app.route('/api/query_profile', methods=['POST'])
def query_profile():
    """查询用户信息: -c -u"""
    data = request.get_json()
    cmd = f"query_profile -c {data['c']} -u {data['u']}"
    output = backend.send_command(cmd)
    return jsonify({"result": remove_timestamp(output)})


@app.route('/api/modify_profile', methods=['POST'])
def modify_profile():
    """修改用户信息: -c -u (-p) (-n) (-m) (-g)"""
    data = request.get_json()
    parts = [f"modify_profile -c {data['c']} -u {data['u']}"]
    if data.get('p'): parts.append(f"-p {data['p']}")
    if data.get('n'): parts.append(f"-n {data['n']}")
    if data.get('m'): parts.append(f"-m {data['m']}")
    if data.get('g'): parts.append(f"-g {data['g']}")
    cmd = " ".join(parts)
    output = backend.send_command(cmd)
    return jsonify({"result": remove_timestamp(output)})


@app.route('/api/add_train', methods=['POST'])
def add_train():
    """添加车次: -i -n -m -s -p -x -t -o -d -y"""
    data = request.get_json()
    cmd = (
        f"add_train "
        f"-i {data['i']} -n {data['n']} -m {data['m']} "
        f"-s {data['s']} -p {data['p']} -x {data['x']} "
        f"-t {data['t']} -o {data['o']} -d {data['d']} -y {data['y']}"
    )
    output = backend.send_command(cmd)
    return jsonify({"result": remove_timestamp(output)})


@app.route('/api/delete_train', methods=['POST'])
def delete_train():
    """删除车次: -i"""
    data = request.get_json()
    cmd = f"delete_train -i {data['i']}"
    output = backend.send_command(cmd)
    return jsonify({"result": remove_timestamp(output)})


@app.route('/api/release_train', methods=['POST'])
def release_train():
    """发布车次: -i"""
    data = request.get_json()
    cmd = f"release_train -i {data['i']}"
    output = backend.send_command(cmd)
    return jsonify({"result": remove_timestamp(output)})


@app.route('/api/query_train', methods=['POST'])
def query_train():
    """查询车次: -i -d"""
    data = request.get_json()
    cmd = f"query_train -i {data['i']} -d {data['d']}"
    output = backend.send_command_multi(cmd)
    lines = output.split('\n')
    if lines:
        lines[0] = remove_timestamp(lines[0])
    return jsonify({"result": "\n".join(lines)})


@app.route('/api/query_ticket', methods=['POST'])
def query_ticket():
    """查询车票: -s -t -d (-p time)"""
    data = request.get_json()
    parts = [f"query_ticket -s {data['s']} -t {data['t']} -d {data['d']}"]
    if data.get('p'): parts.append(f"-p {data['p']}")
    cmd = " ".join(parts)
    output = backend.send_command_multi(cmd)
    lines = output.split('\n')
    if lines:
        lines[0] = remove_timestamp(lines[0])
    return jsonify({"result": "\n".join(lines)})


@app.route('/api/query_transfer', methods=['POST'])
def query_transfer():
    """查询换乘: -s -t -d (-p time)"""
    data = request.get_json()
    parts = [f"query_transfer -s {data['s']} -t {data['t']} -d {data['d']}"]
    if data.get('p'): parts.append(f"-p {data['p']}")
    cmd = " ".join(parts)
    output = backend.send_command_multi(cmd)
    lines = output.split('\n')
    if lines:
        lines[0] = remove_timestamp(lines[0])
    return jsonify({"result": "\n".join(lines)})


@app.route('/api/buy_ticket', methods=['POST'])
def buy_ticket():
    """购票: -u -i -d -n -f -t (-q false)"""
    data = request.get_json()
    parts = [
        f"buy_ticket -u {data['u']} -i {data['i']} -d {data['d']} "
        f"-n {data['n']} -f {data['f']} -t {data['t']}"
    ]
    if data.get('q') == 'true':
        parts.append("-q true")
    cmd = " ".join(parts)
    output = backend.send_command(cmd)
    return jsonify({"result": remove_timestamp(output)})


@app.route('/api/query_order', methods=['POST'])
def query_order():
    """查询订单: -u"""
    data = request.get_json()
    cmd = f"query_order -u {data['u']}"
    output = backend.send_command_multi(cmd)
    lines = output.split('\n')
    if lines:
        lines[0] = remove_timestamp(lines[0])
    return jsonify({"result": "\n".join(lines)})


@app.route('/api/refund_ticket', methods=['POST'])
def refund_ticket():
    """退票: -u (-n 1)"""
    data = request.get_json()
    n = data.get('n', '1')
    cmd = f"refund_ticket -u {data['u']} -n {n}"
    output = backend.send_command(cmd)
    return jsonify({"result": remove_timestamp(output)})


@app.route('/api/clean', methods=['POST'])
def clean():
    """清除所有数据"""
    cmd = "clean"
    output = backend.send_command(cmd)
    return jsonify({"result": remove_timestamp(output)})


# ============ 启动 ============

def shutdown_backend():
    """安全关闭 C++ 后端"""
    backend.stop()


# 注册信号处理（Linux）
if hasattr(signal, 'SIGTERM'):
    signal.signal(signal.SIGTERM, lambda signum, frame: sys.exit(0))
if hasattr(signal, 'SIGINT'):
    signal.signal(signal.SIGINT, lambda signum, frame: sys.exit(0))

# 注册 atexit 确保无论如何都清理后端进程
import atexit
atexit.register(shutdown_backend)


if __name__ == '__main__':
    # 通过环境变量 DEBUG=1 开启调试模式
    debug_mode = os.environ.get('DEBUG', '0') == '1'
    try:
        app.run(host='0.0.0.0', port=5000, debug=debug_mode)
    finally:
        backend.stop()
