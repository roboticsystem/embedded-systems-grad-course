#!/usr/bin/env python3
"""
修复因清理操作导致的表格结构破损：
- 表头行（|...|）与分隔行（|---|---|）之间如果有空行，删除空行使其连续
- 分隔行与数据行之间如果有空行，同理删除
"""
import re
from pathlib import Path

DOCS = Path(__file__).resolve().parents[1] / 'docs'

TABLE_LINE = re.compile(r'^\|.*\|')
TABLE_SEP = re.compile(r'^\|[\s:]*-+[\s:]*\|')
FENCE = re.compile(r'^```')

modified = []

for md in sorted(list(DOCS.glob('chapter*.md')) + list(DOCS.glob('appendix_*.md')) +
                 [DOCS / 'contributing.md', DOCS / 'index.md', DOCS / 'instructor.md']):
    if not md.exists():
        continue
    text = md.read_text(encoding='utf-8')
    lines = text.splitlines(keepends=True)
    n = len(lines)
    to_remove = set()  # indices of blank lines to remove
    in_fence = False

    for i in range(n):
        ln = lines[i].rstrip()
        if FENCE.match(ln):
            in_fence = not in_fence
            continue
        if in_fence:
            continue

        # 找到表头行后面的空行 → 如果空行后面是分隔行，删除空行
        if TABLE_LINE.match(ln) and not TABLE_SEP.match(ln):
            # 这可能是表头行，看后面是否有空行然后分隔行
            j = i + 1
            blank_indices = []
            while j < n and not lines[j].strip():
                blank_indices.append(j)
                j += 1
            if j < n and blank_indices and TABLE_SEP.match(lines[j].rstrip()):
                for bi in blank_indices:
                    to_remove.add(bi)

        # 找到分隔行后面的空行 → 如果空行后面是数据行，删除空行
        if TABLE_SEP.match(ln):
            j = i + 1
            blank_indices = []
            while j < n and not lines[j].strip():
                blank_indices.append(j)
                j += 1
            if j < n and blank_indices and TABLE_LINE.match(lines[j].rstrip()):
                for bi in blank_indices:
                    to_remove.add(bi)

        # 数据行之间的空行也要修复
        if TABLE_LINE.match(ln):
            j = i + 1
            blank_indices = []
            while j < n and not lines[j].strip():
                blank_indices.append(j)
                j += 1
            if j < n and blank_indices and TABLE_LINE.match(lines[j].rstrip()):
                for bi in blank_indices:
                    to_remove.add(bi)

    if to_remove:
        new_lines = [lines[i] for i in range(n) if i not in to_remove]
        md.write_text(''.join(new_lines), encoding='utf-8')
        modified.append((md.name, len(to_remove)))

if modified:
    print('已修复表格：')
    for name, cnt in modified:
        print(f'  {name}: 删除 {cnt} 个空行')
else:
    print('无需修复。')
