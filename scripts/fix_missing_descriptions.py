#!/usr/bin/env python3
"""
为图/表缺少后续描述文本的位置，基于上下文自动生成中文描述段落。

检测逻辑：
  1. 将 markdown 按 heading 切分为 section
  2. 在每个 section 中定位：
     a) Markdown 管道表格块（连续 | 行）
     b) 代码围栏块（```bob / ```plantuml / ```mermaid 等可视化块）
     c) HTML <img> 或 markdown ![]() 图片
  3. 如果这些元素后面（同 section 内）没有紧跟至少一行正文段落，则标记为需要补充描述
  4. 生成描述：根据 section 标题 + 表头列名 / 图类型，生成简短衔接段落

规则：
  - 表格：在表格末尾后插入一段话，概述表格呈现的内容和关键对比
  - 代码围栏图（bob/plantuml/mermaid）：在围栏块后插入描述，说明图示流程/架构
  - 仅处理 "末尾无文本" 的情况（避免对已有描述的位置重复插入）
  - 幂等：通过 <!-- desc-auto --> 标记避免重复

用法：
  python scripts/fix_missing_descriptions.py [--dry-run]
"""
import re, sys
from pathlib import Path

DOCS = Path(__file__).resolve().parents[1] / 'docs'
DRY_RUN = '--dry-run' in sys.argv
MARKER = '<!-- desc-auto -->'

heading_re = re.compile(r'^(#{1,6})\s+(.*)')
pipe_re = re.compile(r'^\s*\|')
fence_re = re.compile(r'^(\s*)```(\w*)')
img_md_re = re.compile(r'!\[.*\]\(.*\)')
img_html_re = re.compile(r'<img\s', re.I)
bold_fig_re = re.compile(r'^\*\*[图表]\s*\d')

# 需要生成描述的可视化围栏类型
VIS_LANGS = {'bob', 'svgbob', 'plantuml', 'mermaid', 'puml'}

def is_blank_or_comment(line):
    s = line.strip()
    return not s or s.startswith('<!--')

def is_text_paragraph(line):
    """判断是否为正文段落（非空、非标题、非表格、非围栏、非图片、非注释、非列表式编号说明标记）"""
    s = line.strip()
    if not s:
        return False
    if s.startswith('#') or s.startswith('|') or s.startswith('```') or s.startswith('<!--'):
        return False
    if img_md_re.match(s) or img_html_re.match(s):
        return False
    if bold_fig_re.match(s):
        return False
    # 正常文本
    return True

def extract_table_headers(lines):
    """从表格行中提取列头"""
    for ln in lines:
        cells = [c.strip() for c in ln.strip().strip('|').split('|')]
        # 跳过分隔行 (--- 行)
        if all(re.match(r'^[-:]+$', c) for c in cells if c):
            continue
        # 跳过含 bob 图元素的行
        if any(c.startswith('+') or c.startswith('|') and len(c) < 3 for c in cells):
            continue
        # 返回非空 cells
        headers = [c for c in cells if c and not re.match(r'^[-:]+$', c)]
        if headers:
            return headers
    return []

_table_counter = 0
_fig_counter = 0

def _clean_title(section_title):
    """清理标题编号"""
    return re.sub(r'^[\d.]+\s*', '', section_title).strip()

def _detect_table_type(headers, table_lines):
    """根据表头和内容推断表格类型：comparison/parameter/step/troubleshoot/reference"""
    h_lower = ' '.join(h.lower() for h in headers)
    all_text = ' '.join(ln for ln in table_lines).lower()
    
    if any(kw in h_lower for kw in ['步骤', 'step', '①', '②', '阶段', '流程', '顺序']):
        return 'step'
    if any(kw in h_lower for kw in ['问题', '故障', '错误', '原因', '解决', '排查', 'issue', 'error', 'fix']):
        return 'troubleshoot'
    if any(kw in h_lower for kw in ['参数', '值', '单位', '规格', '型号', 'param', 'value', 'spec']):
        return 'parameter'
    if any(kw in h_lower for kw in ['对比', '比较', 'vs', '优点', '缺点', '区别', '特性', '特点']):
        return 'comparison'
    # 如果列数 >= 3 且包含多项属性描述，倾向于对比表
    if len(headers) >= 3:
        return 'comparison'
    return 'reference'

def gen_table_desc(section_title, headers, table_lines):
    """根据表头、内容类型和节标题生成多样化的表格描述"""
    global _table_counter
    _table_counter += 1
    title = _clean_title(section_title)
    ttype = _detect_table_type(headers, table_lines) if headers else 'reference'
    
    cols_str = ''
    if headers:
        # 过滤掉纯格式列头（如 --- ）
        clean_headers = [h for h in headers if not re.match(r'^[-:*]+$', h.strip())]
        cols = '、'.join(clean_headers[:3])
        if len(clean_headers) > 3:
            cols += '等'
        cols_str = cols
    
    templates = {
        'comparison': [
            f'上表对{title}中各方案的特性进行了横向对比，便于读者根据实际需求选择最合适的技术路线。{MARKER}',
            f'通过上表的对比可以看出，不同方案在{cols_str}等方面各有优劣，实际选型时应结合具体应用场景综合权衡。{MARKER}' if cols_str else f'上表系统对比了{title}的多种实现方式，帮助读者全面了解各自的优势与局限。{MARKER}',
        ],
        'parameter': [
            f'上表列出了{title}所涉及的关键参数及其典型取值，这些参数的选择直接影响系统的整体性能与稳定性。{MARKER}',
            f'上述参数配置是{title}的典型推荐值，实际工程中可根据硬件条件和性能需求进行适当调整。{MARKER}',
        ],
        'step': [
            f'上表梳理了{title}的主要操作步骤与执行要点，按照该流程逐步实施可有效降低出错概率。{MARKER}',
            f'上述步骤为{title}的标准操作流程，建议严格按顺序执行并在关键节点进行验证。{MARKER}',
        ],
        'troubleshoot': [
            f'上表汇总了{title}中常见的问题现象、可能原因及推荐解决方案，可作为排障时的快速参考手册。{MARKER}',
            f'在实际操作中遇到上述问题时，建议按表中所列的排查方向逐一定位，避免盲目调试浪费时间。{MARKER}',
        ],
        'reference': [
            f'上表对{title}的核心信息进行了结构化整理，读者可根据需要快速查阅相关内容。{MARKER}',
            f'以上内容归纳了{title}的关键要素，为后续深入学习和工程实践提供了参考依据。{MARKER}',
        ],
    }
    
    options = templates.get(ttype, templates['reference'])
    return options[_table_counter % len(options)]

def gen_figure_desc(section_title, fig_type):
    """根据节标题和图类型生成多样化的图描述"""
    global _fig_counter
    _fig_counter += 1
    title = _clean_title(section_title)
    
    templates_by_type = {
        'bob': [
            f'上图以框图形式描绘了{title}的系统架构，清晰呈现了各模块之间的连接关系与信号流向。{MARKER}',
            f'该框图展示了{title}的核心结构，读者可以从中把握各功能单元的层次划分与协作方式。{MARKER}',
            f'上图直观呈现了{title}的组成要素与数据通路，有助于理解系统整体的工作机理。{MARKER}',
        ],
        'svgbob': [
            f'上图以框图形式描绘了{title}的系统架构，清晰呈现了各模块之间的连接关系与信号流向。{MARKER}',
            f'该框图展示了{title}的核心结构，读者可以从中把握各功能单元的层次划分与协作方式。{MARKER}',
            f'上图直观呈现了{title}的组成要素与数据通路，有助于理解系统整体的工作机理。{MARKER}',
        ],
        'plantuml': [
            f'上图以时序/流程的方式展示了{title}的执行过程，各参与者之间的消息传递顺序一目了然。{MARKER}',
            f'从上述流程图可以看出{title}的关键步骤与判断逻辑，这对正确实现相关功能至关重要。{MARKER}',
            f'上图刻画了{title}涉及的主要交互流程，便于理解各环节的时序依赖与因果关系。{MARKER}',
        ],
        'puml': [
            f'上图展示了{title}的处理流程与状态转换，帮助读者掌握核心算法的执行逻辑。{MARKER}',
            f'该流程图概括了{title}的主要计算步骤，为代码实现提供了清晰的设计蓝图。{MARKER}',
        ],
        'mermaid': [
            f'上图以示意图形式展示了{title}的逻辑结构与各组件间的关联关系。{MARKER}',
            f'通过上述示意图，读者可以快速把握{title}的总体设计思路与模块划分策略。{MARKER}',
        ],
    }
    
    options = templates_by_type.get(fig_type, [
        f'上图展示了{title}的核心结构，帮助读者从全局视角理解系统设计。{MARKER}',
    ])
    return options[_fig_counter % len(options)]

def gen_image_desc(section_title):
    """为独立图片生成多样化描述"""
    global _fig_counter
    _fig_counter += 1
    title = _clean_title(section_title)
    options = [
        f'上图展示了{title}的典型应用场景，结合前文的理论说明有助于建立更直观的认识。{MARKER}',
        f'该图像直观呈现了{title}的实际效果，读者可对照正文描述加深对相关概念的理解。{MARKER}',
    ]
    return options[_fig_counter % len(options)]

def process_file(filepath):
    """处理单个 markdown 文件，返回 (new_content, change_count)"""
    text = filepath.read_text(encoding='utf-8')
    
    # 如果已全部标记过，跳过
    if MARKER in text and text.count(MARKER) > 0:
        # 仍需检查未标记的位置
        pass
    
    lines = text.split('\n')
    n = len(lines)
    
    # 收集所有 heading 位置
    headings = []
    for i, ln in enumerate(lines):
        m = heading_re.match(ln)
        if m:
            headings.append((i, len(m.group(1)), m.group(2).strip()))
    headings.append((n, 0, 'EOF'))
    
    insertions = []  # list of (line_index, text_to_insert)
    
    for hi in range(len(headings) - 1):
        sec_start = headings[hi][0]
        sec_end = headings[hi + 1][0]
        sec_title = headings[hi][2]
        
        # 在 section 内扫描元素
        i = sec_start + 1
        while i < sec_end:
            ln = lines[i]
            stripped = ln.strip()
            
            # 跳过已标记
            if MARKER in stripped:
                i += 1
                continue
            
            # === 检测围栏代码块 ===
            fm = fence_re.match(ln)
            if fm:
                lang = fm.group(2).lower()
                # 找闭合围栏
                j = i + 1
                while j < sec_end:
                    if fence_re.match(lines[j]) and lines[j].strip().startswith('```'):
                        # 确认是闭合（不是新开）
                        fm2 = fence_re.match(lines[j])
                        if fm2 and (fm2.group(2) == '' or fm2.group(2) == fm.group(2)):
                            break
                    j += 1
                fence_end = j
                
                if lang in VIS_LANGS:
                    # 检查围栏后是否有正文
                    k = fence_end + 1
                    while k < sec_end and is_blank_or_comment(lines[k]):
                        k += 1
                    
                    has_text_after = False
                    if k < sec_end and is_text_paragraph(lines[k]):
                        has_text_after = True
                    
                    if not has_text_after:
                        desc = gen_figure_desc(sec_title, lang)
                        insertions.append((fence_end + 1, desc))
                
                i = fence_end + 1
                continue
            
            # === 检测管道表格块 ===
            if pipe_re.match(stripped) and stripped.count('|') >= 2:
                table_start = i
                j = i
                while j < sec_end and pipe_re.match(lines[j].strip()) and lines[j].strip():
                    j += 1
                table_end = j - 1  # inclusive last table line
                table_lines = lines[table_start:table_end + 1]
                
                # 检查表格后是否有正文
                k = table_end + 1
                while k < sec_end and is_blank_or_comment(lines[k]):
                    k += 1
                
                has_text_after = False
                if k < sec_end and is_text_paragraph(lines[k]):
                    has_text_after = True
                
                if not has_text_after:
                    headers = extract_table_headers(table_lines)
                    desc = gen_table_desc(sec_title, headers, table_lines)
                    insertions.append((table_end + 1, desc))
                
                i = table_end + 1
                continue
            
            # === 检测图片 ===
            if img_md_re.search(stripped) or img_html_re.search(stripped):
                k = i + 1
                while k < sec_end and is_blank_or_comment(lines[k]):
                    k += 1
                
                has_text_after = False
                if k < sec_end and is_text_paragraph(lines[k]):
                    has_text_after = True
                
                if not has_text_after:
                    desc = gen_image_desc(sec_title)
                    insertions.append((i + 1, desc))
                
                i += 1
                continue
            
            i += 1
    
    if not insertions:
        return text, 0
    
    # 从后往前插入避免偏移
    insertions.sort(key=lambda x: x[0], reverse=True)
    for idx, desc in insertions:
        lines.insert(idx, '')
        lines.insert(idx + 1, desc)
        lines.insert(idx + 2, '')
    
    return '\n'.join(lines), len(insertions)


def main():
    total_changes = 0
    changed_files = []
    
    # 处理 chapter*.md, appendix_*.md, index.md
    targets = sorted(DOCS.glob('chapter*.md')) + sorted(DOCS.glob('appendix_*.md'))
    # 也包含 contributing.md, index.md, instructor.md
    for extra in ['contributing.md', 'index.md', 'instructor.md']:
        p = DOCS / extra
        if p.exists():
            targets.append(p)
    
    for md in targets:
        new_content, count = process_file(md)
        if count > 0:
            if DRY_RUN:
                print(f'  [DRY] {md.name}: 需插入 {count} 处描述')
            else:
                md.write_text(new_content, encoding='utf-8')
                print(f'  [OK]  {md.name}: 已插入 {count} 处描述')
            total_changes += count
            changed_files.append(md.name)
    
    print(f'\n共 {len(changed_files)} 个文件需处理，合计 {total_changes} 处描述{"（dry-run，未实际写入）" if DRY_RUN else "已写入"}')

if __name__ == '__main__':
    main()
