#!/usr/bin/env python3
"""
Insert placeholder descriptive paragraph after table or image blocks that are followed by heading or EOF without an intervening text paragraph.
Marks insertion with <!-- autoplaceholder --> to avoid duplicates and creates .bak backups.
"""
from pathlib import Path
import re

DOCS = Path(__file__).resolve().parents[1] / 'docs'
PLACEHOLDER = '\n说明：下表/下图用于展示关键信息，请在正文中补充对表/图的说明，解释其结论、来源与与上文的联系。（自动占位，请替换为详细描述）\n\n<!-- autoplaceholder -->\n'

image_re = re.compile(r'!\[.*\]\(.*\)')
html_img_re = re.compile(r'<img\s')
pipe_table_re = re.compile(r'\|')
html_table_start = re.compile(r'<table\b', re.I)
heading_re = re.compile(r'^(#{1,6})\s')

modified = []

for md in sorted(DOCS.glob('*.md')):
    lines = md.read_text(encoding='utf-8').splitlines(keepends=True)
    n = len(lines)
    i = 0
    changed = False
    new_lines = list(lines)
    offset = 0
    while i < n:
        ln = lines[i]
        stripped = ln.strip()
        # skip if already contains placeholder nearby
        if '<!-- autoplaceholder -->' in stripped:
            i += 1
            continue
        # detect HTML table
        if html_table_start.search(stripped):
            # find end tag
            j = i
            while j < n and '</table>' not in lines[j]:
                j += 1
            if j < n:
                end_idx = j
            else:
                end_idx = j
            # find next meaningful line
            k = end_idx + 1
            while k < n and (not lines[k].strip() or lines[k].strip().startswith('<!--')):
                k += 1
            if k >= n or heading_re.match(lines[k]):
                # insert placeholder after end_idx
                insert_at = end_idx + 1 + offset
                new_lines.insert(insert_at, PLACEHOLDER)
                offset += 1
                changed = True
            i = end_idx + 1
            continue
        # detect pipe-table block (contiguous lines with |)
        if pipe_table_re.search(stripped) and stripped.count('|')>=2:
            j = i
            while j+1 < n and pipe_table_re.search(lines[j+1]) and lines[j+1].strip():
                j += 1
            end_idx = j
            # find next meaningful
            k = end_idx + 1
            while k < n and (not lines[k].strip() or lines[k].strip().startswith('<!--')):
                k += 1
            if k >= n or heading_re.match(lines[k]):
                insert_at = end_idx + 1 + offset
                new_lines.insert(insert_at, PLACEHOLDER)
                offset += 1
                changed = True
            i = end_idx + 1
            continue
        # detect image line
        if image_re.search(stripped) or html_img_re.search(stripped):
            j = i
            # image might be followed by caption lines but treat similarly: find next meaningful
            k = j + 1
            while k < n and (not lines[k].strip() or lines[k].strip().startswith('<!--')):
                k += 1
            if k >= n or heading_re.match(lines[k]) or image_re.search(lines[k]) or pipe_table_re.search(lines[k]):
                insert_at = j + 1 + offset
                new_lines.insert(insert_at, PLACEHOLDER)
                offset += 1
                changed = True
            i = j + 1
            continue
        i += 1
    if changed:
        bak = md.with_suffix(md.suffix + '.bak')
        md.rename(bak)
        md.write_text(''.join(new_lines), encoding='utf-8')
        modified.append(md.name)

if modified:
    print('Inserted placeholders into:')
    for f in modified:
        print(' -', f)
else:
    print('No insertions needed.')
