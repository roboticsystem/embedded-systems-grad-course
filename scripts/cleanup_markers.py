#!/usr/bin/env python3
"""
清理 auto_number_figures_tables.py 首次运行错误插入的标记。
删除所有 **图 X-Y**、**表 X-Y**、<!-- fig:chX-Y -->、<!-- tab:chX-Y --> 行，
以及残留的 autoplaceholder 说明文字行。
"""
import re
from pathlib import Path

DOCS = Path(__file__).resolve().parents[1] / 'docs'

patterns_to_remove = [
    re.compile(r'^\s*\*\*图\s*[\w]+-\d+\*\*'),
    re.compile(r'^\s*\*\*表\s*[\w]+-\d+\*\*'),
    re.compile(r'^\s*<!--\s*fig:ch[\w]+-\d+'),
    re.compile(r'^\s*<!--\s*tab:ch[\w]+-\d+'),
    re.compile(r'^\s*<!--\s*autoplaceholder\s*-->'),
    re.compile(r'^说明：下表/下图用于展示关键信息'),
]

modified = []

for md in sorted(list(DOCS.glob('chapter*.md')) + list(DOCS.glob('appendix_*.md')) +
                 [DOCS / 'contributing.md', DOCS / 'index.md', DOCS / 'instructor.md']):
    if not md.exists():
        continue
    text = md.read_text(encoding='utf-8')
    lines = text.splitlines(keepends=True)
    new_lines = []
    removed = 0
    for ln in lines:
        stripped = ln.strip()
        if any(p.match(stripped) for p in patterns_to_remove):
            removed += 1
            continue
        new_lines.append(ln)
    if removed > 0:
        new_text = ''.join(new_lines)
        # 清理连续空行（最多2个）
        new_text = re.sub(r'\n{4,}', '\n\n\n', new_text)
        md.write_text(new_text, encoding='utf-8')
        modified.append((md.name, removed))

if modified:
    print('已清理标记：')
    for name, cnt in modified:
        print(f'  {name}: 删除 {cnt} 行')
else:
    print('无需清理。')
