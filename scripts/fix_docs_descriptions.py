#!/usr/bin/env python3
"""
Scan markdown files under docs/ and ensure that any subsection (heading + content) that contains only a single image or a single table has a descriptive paragraph after the image/table.
Adds a Chinese placeholder sentence prompting the author to replace it with a real description.
Backs up modified files with .bak
"""
import re
from pathlib import Path

DOCS = Path(__file__).resolve().parents[1] / 'docs'
PLACEHOLDER = '说明：此处为插图/表格，需在正文中补充对图表的描述，解释其含义、来源与与上文的联系。（自动占位，请替换为详细描述）\n\n<!-- autoplaceholder -->\n'

heading_re = re.compile(r'^(#{1,6})\s')
image_re = re.compile(r'!\[.*\]\(.*\)')
html_img_re = re.compile(r'<img\s')
table_pipe_re = re.compile(r'\|')
html_table_re = re.compile(r'<table\s')

modified_files = []

for md in sorted(DOCS.glob('*.md')):
    text = md.read_text(encoding='utf-8')
    lines = text.splitlines(keepends=True)
    sections = []  # list of (start_idx, end_idx)
    # find heading indices
    heading_idxs = []
    for i, line in enumerate(lines):
        if heading_re.match(line):
            heading_idxs.append(i)
    # add sentinel for end
    heading_idxs.append(len(lines))
    changes = False
    new_lines = list(lines)
    for si in range(len(heading_idxs)-1):
        start = heading_idxs[si]
        end = heading_idxs[si+1]
        # section content lines between start+1 and end-1
        content_lines = lines[start+1:end]
        # strip blank lines and markdown comments
        meaningful = [ln for ln in content_lines if ln.strip() and not ln.strip().startswith('<!--')]
        # skip empty sections
        if not meaningful:
            continue
        # check if already has autoplaceholder marker
        joined = ''.join(content_lines)
        if '<!-- autoplaceholder -->' in joined:
            continue
        # count images and tables among meaningful lines
        img_lines = [ln for ln in meaningful if image_re.search(ln) or html_img_re.search(ln)]
        # crude table detection: if any line contains '|' and a header separator exists or html table
        table_lines = []
        if any(table_pipe_re.search(ln) for ln in meaningful):
            # ensure there's at least one non-trivial pipe line (more than 1 pipe)
            pipe_candidates = [ln for ln in meaningful if table_pipe_re.search(ln) and ln.count('|')>=2]
            if pipe_candidates:
                table_lines = pipe_candidates
        if html_table_re.search(joined):
            table_lines = table_lines or [ln for ln in meaningful if html_table_re.search(ln)]
        # If section has exactly one meaningful item and it's an image or a table (or a single code fence that renders image?)
        if len(meaningful) == 1 and (img_lines or table_lines):
            # find insertion point: after the image/table line(s)
            # find index in new_lines of the last meaningful line
            rel_idx = None
            # search from start+1 to end for the meaningful line
            for j in range(start+1, end):
                if lines[j].strip() and not lines[j].strip().startswith('<!--'):
                    rel_idx = j
            if rel_idx is None:
                continue
            # insert placeholder after rel_idx
            new_lines.insert(rel_idx+1, PLACEHOLDER)
            changes = True
    if changes:
        # backup
        bak = md.with_suffix(md.suffix + '.bak')
        md.rename(bak)
        md.write_text(''.join(new_lines), encoding='utf-8')
        modified_files.append(md.name)

if modified_files:
    print('Modified files:')
    for f in modified_files:
        print(' -', f)
else:
    print('No modifications necessary.')
