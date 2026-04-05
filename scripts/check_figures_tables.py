#!/usr/bin/env python3
"""
Check that each image or table in docs/*.md is followed (within the same section) by at least one descriptive paragraph of text.
Prints a report of suspected violations with file and heading context.
"""
import re
from pathlib import Path

DOCS = Path(__file__).resolve().parents[1] / 'docs'
heading_re = re.compile(r'^(#{1,6})\s')
image_re = re.compile(r'!\[.*\]\(.*\)')
html_img_re = re.compile(r'<img\s')
table_pipe_re = re.compile(r'\|')
html_table_re = re.compile(r'<table\s')

issues = []

for md in sorted(DOCS.glob('*.md')):
    lines = md.read_text(encoding='utf-8').splitlines()
    # find headings and their ranges
    headings = []  # list of (idx, level, title)
    for i, ln in enumerate(lines):
        m = heading_re.match(ln)
        if m:
            level = len(m.group(1))
            title = ln.strip()
            headings.append((i, level, title))
    # add sentinel
    headings.append((len(lines), 0, 'EOF'))
    for hi in range(len(headings)-1):
        start_idx = headings[hi][0]
        end_idx = headings[hi+1][0]
        section_title = headings[hi][2]
        # iterate lines in section
        for i in range(start_idx+1, end_idx):
            ln = lines[i].strip()
            if not ln:
                continue
            # if line is image
            if image_re.search(ln) or html_img_re.search(ln):
                # find next non-empty, non-comment line within section
                next_meaningful = None
                for j in range(i+1, end_idx):
                    l2 = lines[j].strip()
                    if not l2:
                        continue
                    if l2.startswith('<!--'):
                        continue
                    # if it's another image/table or a heading or code fence, mark
                    if image_re.search(l2) or html_img_re.search(l2) or table_pipe_re.search(l2) or l2.startswith('```') or heading_re.match(l2) or html_table_re.search(l2):
                        next_meaningful = ('bad', j, l2)
                        break
                    # otherwise it's good text
                    next_meaningful = ('good', j, l2)
                    break
                if next_meaningful is None:
                    issues.append((md.name, section_title, i+1, 'image', 'no following text'))
                elif next_meaningful[0] == 'bad':
                    issues.append((md.name, section_title, i+1, 'image', 'followed by non-descriptive element: '+next_meaningful[2]))
            # if line looks like table (pipe-rich) or html table
            if table_pipe_re.search(ln) and ln.count('|')>=2:
                # ensure it's not a markdown table header separator
                # find next non-empty line
                next_meaningful = None
                for j in range(i+1, end_idx):
                    l2 = lines[j].strip()
                    if not l2:
                        continue
                    if l2.startswith('<!--'):
                        continue
                    if image_re.search(l2) or html_img_re.search(l2) or table_pipe_re.search(l2) or l2.startswith('```') or heading_re.match(l2) or html_table_re.search(l2):
                        next_meaningful = ('bad', j, l2)
                        break
                    next_meaningful = ('good', j, l2)
                    break
                if next_meaningful is None:
                    issues.append((md.name, section_title, i+1, 'table', 'no following text'))
                elif next_meaningful[0] == 'bad':
                    issues.append((md.name, section_title, i+1, 'table', 'followed by non-descriptive element: '+next_meaningful[2]))

if issues:
    print('Found potential issues:')
    for it in issues:
        print(f"- File: {it[0]} | Section: {it[1]} | Line: {it[2]} | Type: {it[3]} | Issue: {it[4]}")
else:
    print('No issues found.')
