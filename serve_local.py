#!/usr/bin/env python3
"""
本地快速预览脚本
用法：python3 serve_local.py
访问：http://127.0.0.1:8008
文件变更后自动刷新，Ctrl+C 停止。
"""

import subprocess
import sys
import os
import signal
import time
from pathlib import Path

REQUIREMENTS_FILE = Path(__file__).resolve().parent / "requirements.txt"
HOST = "127.0.0.1"
PORT = 8008


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
    if not REQUIREMENTS_FILE.exists():
        print(f"\n❌ 未找到依赖文件: {REQUIREMENTS_FILE}")
        sys.exit(1)

    print(f"⚙️  从 {REQUIREMENTS_FILE.name} 安装依赖（含版本约束）")
    subprocess.check_call(
        [sys.executable, "-m", "pip", "install", "--quiet", "-r", str(REQUIREMENTS_FILE)]
    )
    print("✅ 依赖安装完成\n")

def main():
    install_requirements()
    ensure_port_available(HOST, PORT)

    print("=" * 50)
    print("  本地预览服务器")
    print("=" * 50)
    print(f"🌐 地址: http://{HOST}:{PORT}")
    print("🔄 文件保存后自动刷新")
    print("⛔ Ctrl+C 停止\n")

    try:
        env = os.environ.copy()
        # mkdocs-material 9.7.x prints this banner unconditionally; keep logs clean locally.
        env.setdefault("NO_MKDOCS_2_WARNING", "1")
        subprocess.run(
            ["mkdocs", "serve", "-a", f"{HOST}:{PORT}", "--open", "--watch-theme"],
            env=env,
            check=True,
        )
    except KeyboardInterrupt:
        print("\n\n✅ 预览服务器已停止")
    except subprocess.CalledProcessError as e:
        print(f"\n❌ 启动失败: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
