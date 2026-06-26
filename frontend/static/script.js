// ========== 火车票管理系统 - 前端交互脚本 ==========

// ========== 面板切换 ==========
function switchPanel(panelName) {
    document.querySelectorAll('.panel').forEach(p => p.classList.remove('active'));
    const target = document.getElementById('panel-' + panelName);
    if (target) target.classList.add('active');

    document.querySelectorAll('.nav-btn').forEach(b => b.classList.remove('active'));
    const activeBtn = document.querySelector(`[data-panel="${panelName}"]`);
    if (activeBtn) activeBtn.classList.add('active');

    document.querySelector('.main-content').scrollTop = 0;
}

document.querySelectorAll('.nav-btn[data-panel]').forEach(btn => {
    btn.addEventListener('click', () => switchPanel(btn.dataset.panel));
});

// ========== 首个用户开关 ==========
window.toggleFirstUser = function () {
    const checked = document.getElementById('add_user_first').checked;
    document.getElementById('add_user_c').style.display = checked ? 'none' : 'block';
    document.getElementById('add_user_g').style.display = checked ? 'none' : 'block';
};

// ========== 表单提交处理 ==========
document.querySelectorAll('.api-form').forEach(form => {
    form.addEventListener('submit', async (e) => {
        e.preventDefault();

        const api = form.dataset.api;
        const submitBtn = form.querySelector('button[type="submit"]');
        const resultOutput = document.getElementById('resultOutput');

        // 收集表单数据
        const formData = new FormData(form);
        const data = {};
        for (const [key, value] of formData.entries()) {
            if (value.trim() !== '') {
                data[key] = value.trim();
            }
        }

        // 特殊处理：首个用户注册
        if (api === '/api/add_user' && document.getElementById('add_user_first')?.checked) {
            data.first_user = true;
        }

        // 禁用按钮
        if (submitBtn) {
            submitBtn.disabled = true;
            submitBtn.textContent = '执行中...';
        }
        resultOutput.textContent = '⏳ 正在发送请求...';
        resultOutput.className = '';

        try {
            const response = await fetch(api, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(data)
            });

            const result = await response.json();
            resultOutput.textContent = result.result || '(空)';

            // 根据结果显示样式
            const r = result.result;
            if (r === '0' || r === 'bye') {
                resultOutput.className = 'success';
            } else if (r === '-1') {
                resultOutput.className = 'error';
            } else if (r === 'queue') {
                resultOutput.className = 'success';
            } else if (r.startsWith('-1')) {
                resultOutput.className = 'error';
            } else {
                resultOutput.className = 'success';
            }
        } catch (err) {
            resultOutput.textContent = '❌ 请求失败: ' + err.message;
            resultOutput.className = 'error';
        } finally {
            if (submitBtn) {
                submitBtn.disabled = false;
                const panelId = form.closest('.panel')?.id?.replace('panel-', '') || '';
                const btnTexts = {
                    'add_user': '注册', 'login': '登录', 'logout': '登出',
                    'query_profile': '查询', 'modify_profile': '修改',
                    'add_train': '添加', 'delete_train': '删除', 'release_train': '发布',
                    'query_train': '查询', 'query_ticket': '查询', 'query_transfer': '查询换乘',
                    'buy_ticket': '购买', 'query_order': '查询订单', 'refund_ticket': '退票',
                    'clean': '确认清除'
                };
                submitBtn.textContent = submitBtn.classList.contains('btn-danger')
                    ? (panelId === 'clean' ? '确认清除' : (panelId === 'delete_train' ? '删除' : '退票'))
                    : (btnTexts[panelId] || '提交');
            }
        }
    });
});

// ========== 重启后端 ==========
document.getElementById('restartBackend').addEventListener('click', async () => {
    const output = document.getElementById('resultOutput');
    output.textContent = '⏳ 正在重启后端...';
    output.className = '';
    try {
        const resp = await fetch('/api/restart');
        const result = await resp.json();
        output.textContent = result.result;
        output.className = 'success';
        checkStatus();
    } catch (err) {
        output.textContent = '❌ 重启失败: ' + err.message;
        output.className = 'error';
    }
});

// ========== 检查后端状态 ==========
async function checkStatus() {
    try {
        const resp = await fetch('/api/status');
        const data = await resp.json();
        const dot = document.querySelector('.status-dot');
        const text = document.getElementById('statusText');
        if (data.status === 'running') {
            dot.className = 'status-dot online';
            text.textContent = '后端运行中';
        } else {
            dot.className = 'status-dot offline';
            text.textContent = '后端未运行';
        }
    } catch {
        document.querySelector('.status-dot').className = 'status-dot offline';
        document.getElementById('statusText').textContent = '连接失败';
    }
}

checkStatus();
setInterval(checkStatus, 10000);

// ========== Ctrl+Enter 提交表单 ==========
document.addEventListener('keydown', (e) => {
    if (e.ctrlKey && e.key === 'Enter') {
        const activePanel = document.querySelector('.panel.active');
        if (activePanel) {
            const form = activePanel.querySelector('.api-form');
            if (form) form.dispatchEvent(new Event('submit'));
        }
    }
});
