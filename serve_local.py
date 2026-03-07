#!/usr/bin/env python3
"""
本地快速预览脚本
用法：python3 serve_local.py
访问：
  http://127.0.0.1:8008  —— MkDocs 文档（热重载）
  http://127.0.0.1:8009  —— 考试 API + 教师/查分页面
  http://127.0.0.1:8009/teacher  —— 教师管理后台
  http://127.0.0.1:8009/score    —— 学生查分页面
文件变更后自动刷新，Ctrl+C 停止。
"""

import subprocess
import sys
import os
import signal
import time
import tempfile
from pathlib import Path

REPO_ROOT          = Path(__file__).resolve().parent
REQUIREMENTS_FILE  = REPO_ROOT / "requirements.txt"
BACKEND_REQ_FILE   = REPO_ROOT / "backend" / "requirements.txt"
BACKEND_DIR        = REPO_ROOT / "backend"

HOST        = "127.0.0.1"
MKDOCS_PORT = 8008
API_PORT    = 8009


def _collect_pids_on_port(port: int):
    """Return PIDs listening on the given TCP port using common Linux tools."""
    pid_set = set()

    commands = [
        ["lsof", "-ti", f"tcp:{port}"],
        ["fuser", "-n", "tcp", str(port)],
    ]

    for cmd in commands:
        try:
            res = subprocess.run(cmd, capture_output=True, text=True, check=False)
        except FileNotFoundError:
            continue

        text = (res.stdout or "") + " " + (res.stderr or "")
        for token in text.replace("\n", " ").split():
            if token.isdigit():
                pid_set.add(int(token))

    return sorted(pid_set)


def _is_port_busy(host: str, port: int) -> bool:
    import socket

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        try:
            sock.bind((host, port))
            return False
        except OSError:
            return True


def ensure_port_available(host: str, port: int):
    if not _is_port_busy(host, port):
        return

    print(f"⚠️  端口 {port} 已被占用，尝试清理旧预览进程...")
    pids = _collect_pids_on_port(port)

    if not pids:
        print("❌ 无法识别占用进程，请手动释放端口后重试")
        sys.exit(1)

    for pid in pids:
        try:
            os.kill(pid, signal.SIGTERM)
            print(f"  - 已发送 SIGTERM 给 PID {pid}")
        except ProcessLookupError:
            continue
        except PermissionError:
            print(f"❌ 无权限结束 PID {pid}")
            sys.exit(1)

    time.sleep(1)

    if _is_port_busy(host, port):
        for pid in pids:
            try:
                os.kill(pid, signal.SIGKILL)
                print(f"  - 已发送 SIGKILL 给 PID {pid}")
            except ProcessLookupError:
                continue
            except PermissionError:
                print(f"❌ 无权限强制结束 PID {pid}")
                sys.exit(1)
        time.sleep(0.5)

    if _is_port_busy(host, port):
        print(f"❌ 端口 {port} 仍被占用，请手动检查后重试")
        sys.exit(1)

    print(f"✅ 端口 {port} 已释放")


def install_requirements():
    for req_file in [REQUIREMENTS_FILE, BACKEND_REQ_FILE]:
        if not req_file.exists():
            print(f"\n❌ 未找到依赖文件: {req_file}")
            sys.exit(1)
        print(f"⚙️  安装依赖：{req_file.relative_to(REPO_ROOT)}")
        subprocess.check_call(
            [sys.executable, "-m", "pip", "install", "--quiet", "-r", str(req_file)]
        )
    print("✅ 依赖安装完成\n")


def start_api_server():
    """在后台启动 FastAPI 考试后端，返回 Popen 对象。"""
    data_dir = REPO_ROOT / ".local_exam_data"
    data_dir.mkdir(exist_ok=True)

    env = os.environ.copy()
    env.setdefault("DB_PATH",           str(data_dir / "exam.db"))
    env.setdefault("TEACHER_PASSWORD",  "admin123")
    env.setdefault("JWT_SECRET",        "local-dev-secret-not-for-production")
    env["PYTHONPATH"] = str(BACKEND_DIR)

    proc = subprocess.Popen(
        [
            sys.executable, "-m", "uvicorn", "app.main:app",
            "--host", HOST,
            "--port", str(API_PORT),
            "--reload",
        ],
        cwd=str(BACKEND_DIR),
        env=env,
    )
    return proc


def main():
    install_requirements()
    ensure_port_available(HOST, MKDOCS_PORT)
    ensure_port_available(HOST, API_PORT)

    print("=" * 55)
    print("  本地预览服务器（MkDocs + 考试后端）")
    print("=" * 55)
    print(f"📖 文档地址：  http://{HOST}:{MKDOCS_PORT}  （热重载）")
    print(f"🎓 教师后台：  http://{HOST}:{API_PORT}/teacher  （密码：admin123）")
    print(f"📊 学生查分：  http://{HOST}:{API_PORT}/score")
    print(f"📡 API 文档：  http://{HOST}:{API_PORT}/api/docs")
    print("⛔ Ctrl+C 停止所有服务\n")

    api_proc = start_api_server()

    # 等待 API 服务就绪（最多 10 秒）
    import socket as _socket
    for _ in range(20):
        time.sleep(0.5)
        try:
            with _socket.create_connection((HOST, API_PORT), timeout=0.3):
                break
        except OSError:
            continue
    else:
        print(f"⚠️  考试 API 服务未能在 10 秒内启动，请检查日志")

    try:
        env = os.environ.copy()
        env.setdefault("NO_MKDOCS_2_WARNING", "1")
        subprocess.run(
            ["mkdocs", "serve", "-a", f"{HOST}:{MKDOCS_PORT}", "--open", "--watch-theme"],
            env=env,
            check=True,
            cwd=str(REPO_ROOT),
        )
    except KeyboardInterrupt:
        print("\n\n⛔ 正在停止所有服务...")
    except subprocess.CalledProcessError as e:
        print(f"\n❌ MkDocs 启动失败: {e}")
    finally:
        api_proc.terminate()
        try:
            api_proc.wait(timeout=3)
        except subprocess.TimeoutExpired:
            api_proc.kill()
        print("✅ 所有服务已停止")


if __name__ == "__main__":
    main()

