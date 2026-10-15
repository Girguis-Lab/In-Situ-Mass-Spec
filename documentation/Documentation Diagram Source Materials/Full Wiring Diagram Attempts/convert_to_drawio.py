"""
Converts blocks.csv + wire_links_with_pins.csv into a native draw.io (mxGraph) XML file.

No PlantUML involved -- this emits mxCell vertices/edges directly:
  - groups/subgroups  -> swimlane containers (draw.io's closest native equivalent of a UML package)
  - plain components  -> rectangle vertices
  - pinned connectors -> hexagon container vertices with one small ellipse child per pin
  - wires             -> edges colored/dashed to match wire_links_with_pins.csv

Layout is intentionally naive: blocks/groups are placed left-to-right / top-to-bottom using
running coordinate cursors (no crossing minimization, no auto-routing). Open the result in
draw.io and run Arrange > Layout for a nicer arrangement -- geometry here only needs to be
non-degenerate, not pretty.
"""

import csv
import random
import re
import sys
from collections import OrderedDict
from xml.sax.saxutils import escape, quoteattr

random.seed(42)  # reproducible jitter across runs

BASE = "/Users/ky/Documents/Girguis Lab/ClaudeCode"

with open(f"{BASE}/blocks.csv", newline="", encoding="utf-8") as f:
    blocks = list(csv.DictReader(f))

with open(f"{BASE}/wire_links_with_pins.csv", newline="", encoding="utf-8") as f:
    wires = list(csv.DictReader(f))

# --- Hardcoded alias map for blocks (name -> id) ---
ALIAS = {
    "LAPTOP": "LAPTOP",
    "USB to RS232 (to RGA COMMS DB9)": "USB_RGA",
    "USB to RS232 (to MAIN COMMS DB9)": "USB_MAIN",
    "RGA COMMS DB9 (with fake RS232 handshake jumpers)": "RGA_COMMS_DB9",
    "MAIN COMMS DB9": "MAIN_COMMS_DB9",
    "POWER SUPPLY A (24-48v @ 4A)": "PS_A",
    "POWER SUPPLY B (24-48v @ 4A)": "PS_B",
    "Topside connection": "TOPSIDE_CONN",
    "SubConn 8-pin (M)": "SC8",
    "SubConn 16-pin": "SC16",
    "FLUID PUMP (12v power, 5v signals, SubConn 5-pin)": "FLUIDPUMP",
    "pH LOGGER (Optional, 5v, SubConn 6-pin)": "PHLOGGER",
    "Main PCB": "MAINPCB",
    "ROUGHING PUMP (24v)": "ROUGHPUMP",
    "DB9 (with autostart jumpers)": "DB9_AUTOSTART",
    "TURBO PUMP (24v)": "TURBOPUMP",
    "DB15": "DB15DEV",
    "RGA (24v)": "RGADEV",
    "DB9 (with fake RS232 handshake jumpers)": "DB9_RGA",
    "Barrel Jack": "BARRELJACK",
}

for b in blocks:
    n = b["Block"].strip()
    if n not in ALIAS:
        print(f"MISSING ALIAS FOR: {n!r}", file=sys.stderr)

# --- Which blocks have numbered pins referenced in the wire list ---
pin_pattern = re.compile(r"^(.*)\s\[(\d+)\]$")

pin_counts = {}  # alias -> max pin number seen
for w in wires:
    for field in ("From", "To"):
        val = w[field].strip()
        if val == "N/A":
            continue
        m = pin_pattern.match(val)
        if m:
            base = m.group(1).strip()
            pin = int(m.group(2))
            if base not in ALIAS:
                print(f"MISSING ALIAS FOR PIN BASE: {base!r}", file=sys.stderr)
                continue
            alias = ALIAS[base]
            pin_counts[alias] = max(pin_counts.get(alias, 0), pin)

# --- Block info: alias -> name/color/group ---
info = {}
for b in blocks:
    name = b["Block"].strip()
    alias = ALIAS[name]
    info[alias] = {
        "name": name,
        "color": b["Block Color"].strip().lower(),
        "group": b["Associated Component Group"].strip(),
    }

COLOR_HEX = {
    "gray": "#808080",
    "grey": "#808080",
    "red": "#FF0000",
    "black": "#000000",
    "white": "#FFFFFF",
    "blue": "#0000FF",
    "green": "#008000",
    "yellow": "#FFFF00",
    "orange": "#FFA500",
    "magenta": "#FF00FF",
    "brown": "#A52A2A",
}

def hex_color(name):
    return COLOR_HEX.get(name.strip().lower(), "#CCCCCC")

# --- Group hierarchy (same grouping rules as the PlantUML version) ---
group_children = OrderedDict()  # group_path tuple -> [alias, ...] direct members
group_order = []

for alias, d in info.items():
    path = tuple(p.strip() for p in d["group"].split(">"))
    if path not in group_children:
        group_children[path] = []
        group_order.append(path)
    group_children[path].append(alias)

TOP_ORDER = ["Topside", "External", "Main Housing"]

def children_of(prefix):
    seen = []
    for path in group_order:
        if len(path) == len(prefix) + 1 and path[: len(prefix)] == prefix and path not in seen:
            seen.append(path)
    return seen

def pkg_id(path):
    return "PKG_" + "_".join(re.sub(r"[^A-Za-z0-9]+", "", p).upper() for p in path)

def resolve_endpoint(val):
    val = val.strip()
    m = pin_pattern.match(val)
    if m:
        base = m.group(1).strip()
        return f"{ALIAS[base]}_{m.group(2)}"
    return ALIAS[val]

def base_alias(pin_alias):
    for pinned in pin_counts:
        if pin_alias == pinned or pin_alias.startswith(pinned + "_"):
            return pinned
    return pin_alias

# --- Stub nodes for wires with an "N/A" endpoint (unused/unconnected pins) ---
stub_by_group = OrderedDict()  # group_path -> [stub_id, ...]
stub_edges = []  # (source_id, target_id, wire_color, wire_stripe, label)
stub_counter = 0

for w in wires:
    frm = w["From"].strip()
    to = w["To"].strip()
    if frm != "N/A" and to != "N/A":
        continue
    stub_counter += 1
    stub_id = f"STUB{stub_counter}"
    label = w["Wire Label"].strip()
    if frm == "N/A":
        real_alias = resolve_endpoint(to)
        group_path = tuple(p.strip() for p in info[base_alias(real_alias)]["group"].split(">"))
        stub_edges.append((stub_id, real_alias, w["Wire Color"], w["Wire Stripe Color"], label))
    else:
        real_alias = resolve_endpoint(frm)
        group_path = tuple(p.strip() for p in info[base_alias(real_alias)]["group"].split(">"))
        stub_edges.append((real_alias, stub_id, w["Wire Color"], w["Wire Stripe Color"], label))
    stub_by_group.setdefault(group_path, []).append(stub_id)

# ============================== layout (sequential, no optimization) ==============================
PAD = 20
HEADER = 30       # swimlane title-bar height
RECT_W, RECT_H = 200, 60
PIN_D = 30
PIN_GAP = 10
HEX_PAD_X = 30
HEX_H = 140
CHILD_GAP = 40
ROW_GAP = 30
GROUP_GAP = 60
STUB_SIZE = 10
Y_JITTER_STEP = 6  # px spacing between the distinct shuffled y-offsets given to blocks in a row

def block_size(alias):
    if alias in pin_counts:
        n = pin_counts[alias]
        w = HEX_PAD_X * 2 + (n - 1) * (PIN_D + PIN_GAP) + PIN_D
        return max(w, 220), HEX_H
    return RECT_W, RECT_H

def layout_group(path):
    """Recursively computes a naive left-to-right / top-to-bottom layout for a group.
    Returns a dict describing this group's own size plus the relative placement of
    everything inside it (its coordinate children are relative to ITS own origin)."""
    direct = group_children.get(path, [])
    stubs = stub_by_group.get(path, [])
    subgroups = children_of(path)

    row1 = []  # (kind, alias/id, x, y, w, h)
    x_cursor = PAD
    row1_top = HEADER + PAD
    max_bottom = row1_top
    # Slight y-jitter per block, drawn from a shuffled set of distinct offsets so no two
    # blocks in this row can coincidentally land on the exact same y (a plain random draw
    # per block risks collisions once a row has more than a couple of items). Never negative,
    # so it never encroaches on the header. Connector pins are positioned separately (in
    # emit_block) and are never jittered.
    jitter_offsets = list(range(0, len(direct) * Y_JITTER_STEP, Y_JITTER_STEP))
    random.shuffle(jitter_offsets)
    for alias, jitter in zip(direct, jitter_offsets):
        w, h = block_size(alias)
        y = row1_top + jitter
        row1.append(("block", alias, x_cursor, y, w, h))
        x_cursor += w + CHILD_GAP
        max_bottom = max(max_bottom, y + h)
    for sid in stubs:
        row1.append(("stub", sid, x_cursor, row1_top, STUB_SIZE, STUB_SIZE))
        x_cursor += STUB_SIZE + CHILD_GAP
        max_bottom = max(max_bottom, row1_top + STUB_SIZE)
    row1_w = (x_cursor - CHILD_GAP + PAD) if (direct or stubs) else 0
    row1_h = max_bottom - row1_top

    row2_y = HEADER + PAD + (row1_h + ROW_GAP if (direct or stubs) else 0)
    row2 = []  # (subgroup_result, x, y)
    x_cursor2 = PAD
    row2_h = 0
    for sp in subgroups:
        sub_res = layout_group(sp)
        row2.append((sub_res, x_cursor2, row2_y))
        x_cursor2 += sub_res["w"] + CHILD_GAP
        row2_h = max(row2_h, sub_res["h"])
    row2_w = (x_cursor2 - CHILD_GAP + PAD) if subgroups else 0

    total_w = max(row1_w, row2_w, 220)
    if subgroups:
        total_h = row2_y + row2_h + PAD
    elif direct or stubs:
        total_h = HEADER + PAD + row1_h + PAD
    else:
        total_h = HEADER + PAD

    return {
        "path": path,
        "id": pkg_id(path),
        "label": path[-1],
        "w": total_w,
        "h": total_h,
        "row1": row1,
        "row2": row2,
    }

# ============================== XML emission ==============================
cells = []  # raw <mxCell ...>...</mxCell> strings

def add_vertex(cid, parent, x, y, w, h, style, value=""):
    cells.append(
        f'<mxCell id={quoteattr(cid)} value={quoteattr(value)} style={quoteattr(style)} '
        f'vertex="1" parent={quoteattr(parent)}>'
        f'<mxGeometry x="{x}" y="{y}" width="{w}" height="{h}" as="geometry"/>'
        f"</mxCell>"
    )

def add_edge(eid, parent, source, target, style, value=""):
    cells.append(
        f'<mxCell id={quoteattr(eid)} value={quoteattr(value)} style={quoteattr(style)} '
        f'edge="1" parent={quoteattr(parent)} source={quoteattr(source)} target={quoteattr(target)}>'
        f'<mxGeometry relative="1" as="geometry"/>'
        f"</mxCell>"
    )

def emit_block(alias, parent, x, y, w, h):
    d = info[alias]
    color = hex_color(d["color"])
    if alias in pin_counts:
        style = (
            f"shape=hexagon;perimeter=hexagonPerimeter2;whiteSpace=wrap;html=1;"
            f"fillColor={color};container=1;collapsible=0;verticalLabelPosition=top;"
            f"verticalAlign=bottom;fontStyle=1;"
        )
        add_vertex(alias, parent, x, y, w, h, style, d["name"])
        n = pin_counts[alias]
        px = HEX_PAD_X
        py = (h - PIN_D) / 2
        for i in range(1, n + 1):
            pin_style = "ellipse;whiteSpace=wrap;html=1;fillColor=#FFFFFF;fontSize=10;"
            add_vertex(f"{alias}_{i}", alias, px, py, PIN_D, PIN_D, pin_style, str(i))
            px += PIN_D + PIN_GAP
    else:
        style = f"rounded=0;whiteSpace=wrap;html=1;fillColor={color};"
        add_vertex(alias, parent, x, y, w, h, style, d["name"])

def emit_stub(sid, parent, x, y, w, h):
    style = "rounded=0;whiteSpace=wrap;html=1;fillColor=none;strokeColor=none;fontColor=none;"
    add_vertex(sid, parent, x, y, w, h, style, "")

def emit_group(res, parent, x, y):
    gid = res["id"]
    style = (
        "swimlane;whiteSpace=wrap;html=1;fillColor=#FFD1DC;swimlaneFillColor=#FFD1DC;"
        "startSize=30;fontStyle=1;container=1;collapsible=0;"
    )
    add_vertex(gid, parent, x, y, res["w"], res["h"], style, res["label"])
    for kind, ident, cx, cy, cw, ch in res["row1"]:
        if kind == "block":
            emit_block(ident, gid, cx, cy, cw, ch)
        else:
            emit_stub(ident, gid, cx, cy, cw, ch)
    for sub_res, cx, cy in res["row2"]:
        emit_group(sub_res, gid, cx, cy)

# --- place the three top-level groups stacked vertically (sequential coordinates) ---
cursor_y = 40
for g in TOP_ORDER:
    res = layout_group((g,))
    emit_group(res, "1", 40, cursor_y)
    cursor_y += res["h"] + GROUP_GAP

# --- wires ---
edge_counter = 0
for w in wires:
    frm = w["From"].strip()
    to = w["To"].strip()
    if frm == "N/A" or to == "N/A":
        continue
    edge_counter += 1
    source = resolve_endpoint(frm)
    target = resolve_endpoint(to)
    color = hex_color(w["Wire Color"])
    stripe = w["Wire Stripe Color"].strip()
    style = f"endArrow=block;html=1;strokeColor={color};strokeWidth=3;rounded=0;"
    if stripe:
        style += "dashed=1;"
    add_edge(f"E{edge_counter}", "1", source, target, style, w["Wire Label"].strip())

for source, target, color, stripe, label in stub_edges:
    edge_counter += 1
    style = f"endArrow=block;html=1;strokeColor={hex_color(color)};strokeWidth=3;rounded=0;"
    if stripe.strip():
        style += "dashed=1;"
    add_edge(f"E{edge_counter}", "1", source, target, style, label)

# ============================== assemble the .drawio file ==============================
xml_parts = []
xml_parts.append('<?xml version="1.0" encoding="UTF-8"?>')
xml_parts.append('<mxfile host="app.diagrams.net">')
xml_parts.append('<diagram name="ISMS Wiring" id="isms-wiring">')
xml_parts.append(
    '<mxGraphModel dx="1600" dy="900" grid="1" gridSize="10" guides="1" tooltips="1" '
    'connect="1" arrows="1" fold="1" page="1" pageScale="1" pageWidth="1600" pageHeight="1200" '
    'math="0" shadow="0" background="#FFD1DC">'
)
xml_parts.append("<root>")
xml_parts.append('<mxCell id="0"/>')
xml_parts.append('<mxCell id="1" parent="0"/>')
xml_parts.extend(cells)
xml_parts.append("</root>")
xml_parts.append("</mxGraphModel>")
xml_parts.append("</diagram>")
xml_parts.append("</mxfile>")

out_path = f"{BASE}/wiring.drawio"
with open(out_path, "w", encoding="utf-8") as f:
    f.write("\n".join(xml_parts))

print(f"Wrote {out_path}", file=sys.stderr)
print(f"Vertices+edges emitted: {len(cells)}", file=sys.stderr)
print(f"Stub wires added: {len(stub_edges)}", file=sys.stderr)
