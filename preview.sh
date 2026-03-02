#!/usr/bin/env bash
# 本地预览 MkDocs 站点，Ctrl+C 停止
set -e

cd "$(dirname "$0")"

# 检查 mkdocs
if ! command -v mkdocs &>/dev/null; then
    echo "❌ 未找到 mkdocs，请先安装："
    echo "   pip install mkdocs mkdocs-material plantuml-markdown jieba"
    exit 1
fi

echo "================================================"
echo "  本地预览：http://127.0.0.1:8000"
echo "  文件修改后自动刷新，Ctrl+C 退出"
echo "================================================"

# 自动打开浏览器（如果有图形界面）
if command -v xdg-open &>/dev/null; then
    sleep 1 && xdg-open http://127.0.0.1:8000 &
elif command -v open &>/dev/null; then
    sleep 1 && open http://127.0.0.1:8000 &
fi

mkdocs serve --dev-addr 127.0.0.1:8000
