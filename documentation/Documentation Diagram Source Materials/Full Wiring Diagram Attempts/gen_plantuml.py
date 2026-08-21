import csv
import re
import sys
from collections import OrderedDict

BASE = "/Users/ky/Documents/Girguis Lab/ClaudeCode"

with open(f"{BASE}/blocks.csv", newline="", encoding="utf-8") as f:
    blocks = list(csv.DictReader(f))

with open(f"{BASE}/wire_links_with_pins.csv", newline="", encoding="utf-8") as f:
    wires = list(csv.DictReader(f))

# --- Hardcoded alias map for blocks (name -> alias) ---
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

# sanity check all block names are mapped
block_names = [b["Block"].strip() for b in blocks]
for n in block_names:
    if n not in ALIAS:
        print(f"MISSING ALIAS FOR: {n!r}", file=sys.stderr)

# --- Determine which blocks have numbered pins referenced in wires ---
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

print("Blocks with pins (hexagon+circles):", pin_counts, file=sys.stderr)

# --- Build block info dict: alias -> (name, color, group) ---
info = {}
for b in blocks:
    name = b["Block"].strip()
    alias = ALIAS[name]
    info[alias] = {
        "name": name,
        "color": b["Block Color"].strip().lower(),
        "group": b["Associated Component Group"].strip(),
    }

# --- Group hierarchy ---
# top-level order: Topside, External, Main Housing
# collect children per group path
group_children = OrderedDict()  # group_path (tuple) -> list of alias (direct members)
group_order = []  # order of first appearance of each group path

for alias, d in info.items():
    path = tuple(p.strip() for p in d["group"].split(">"))
    if path not in group_children:
        group_children[path] = []
        group_order.append(path)
    group_children[path].append(alias)

TOP_ORDER = ["Topside", "External", "Main Housing"]

def children_of(prefix):
    """Return direct sub-group paths (length prefix+1) that start with prefix, in first-seen order."""
    seen = []
    for path in group_order:
        if len(path) == len(prefix) + 1 and path[:len(prefix)] == prefix:
            if path not in seen:
                seen.append(path)
    return seen

def pkg_alias(path):
    return "PKG_" + "_".join(re.sub(r"[^A-Za-z0-9]+", "", p).upper() for p in path)

# --- Wire color -> plantuml style ---
def style_for(wire_color, stripe_color):
    wc = wire_color.strip().lower()
    sc = stripe_color.strip().lower()
    style = f"#{wc}"
    if sc:
        style += f";#{sc}"
    extra = ",thickness=3"
    if sc:
        extra += ",dashed"
    return f"-[{style}{extra}]->"

def resolve_endpoint(val):
    val = val.strip()
    m = pin_pattern.match(val)
    if m:
        base = m.group(1).strip()
        pin = m.group(2)
        alias = ALIAS[base]
        return f"{alias}_{pin}"
    else:
        return ALIAS[val]

def base_alias(pin_alias):
    # strip trailing _<digits> pin suffix if this alias belongs to a pinned block
    for pinned_alias in pin_counts:
        if pin_alias == pinned_alias or pin_alias.startswith(pinned_alias + "_"):
            return pinned_alias
    return pin_alias

# top-level group index for each block alias, based on its group path
TOP_INDEX = {}
for alias, d in info.items():
    top_group = d["group"].split(">")[0].strip()
    TOP_INDEX[alias] = TOP_ORDER.index(top_group)

# --- Stub nodes for wires with an "N/A" endpoint (unused/unconnected pins) ---
# Rendered as an invisible node inside the same group as the real (non-N/A) endpoint,
# so the wire shows up as a dangling colored stub instead of being dropped.
stub_by_group = OrderedDict()  # group_path -> list of stub aliases to declare
stub_edges = []  # (source_alias, target_alias, style, label) using stub alias on one end
stub_counter = 0

for w in wires:
    frm = w["From"].strip()
    to = w["To"].strip()
    if frm != "N/A" and to != "N/A":
        continue
    stub_counter += 1
    stub_alias = f"STUB{stub_counter}"
    style = style_for(w["Wire Color"], w["Wire Stripe Color"])
    label = w["Wire Label"].strip()
    if frm == "N/A":
        real_alias = resolve_endpoint(to)
        group_path = tuple(p.strip() for p in info[base_alias(real_alias)]["group"].split(">"))
        stub_edges.append((stub_alias, real_alias, style, label))
    else:
        real_alias = resolve_endpoint(frm)
        group_path = tuple(p.strip() for p in info[base_alias(real_alias)]["group"].split(">"))
        stub_edges.append((real_alias, stub_alias, style, label))
    stub_by_group.setdefault(group_path, []).append(stub_alias)

lines = []
indent_unit = "  "

def emit(s, depth):
    lines.append(indent_unit * depth + s)

def render_block(alias, depth):
    d = info[alias]
    name = d["name"]
    color = d["color"]
    if alias in pin_counts:
        n = pin_counts[alias]
        emit(f'hexagon "{name}" as {alias} #{color} {{', depth)
        for i in range(1, n + 1):
            emit(f'circle "{i}" as {alias}_{i}', depth + 1)
        emit("}", depth)
    else:
        emit(f'rectangle "{name}" as {alias} #{color}', depth)

def render_group(path, depth):
    label = path[-1]
    alias = pkg_alias(path)
    emit(f'package "{label}" as {alias} {{', depth)
    # direct member blocks
    for m in group_children.get(path, []):
        render_block(m, depth + 1)
    # stub nodes belonging directly to this group
    for stub_alias in stub_by_group.get(path, []):
        emit(f'rectangle " " as {stub_alias} <<stub>>', depth + 1)
    # sub-groups
    for child_path in children_of(path):
        render_group(child_path, depth + 1)
    emit("}", depth)
    return alias

out = []
out.append("@startuml")
out.append("!pragma layout elk")
out.append("skinparam wrapWidth 200")
out.append("skinparam defaultTextAlignment center")
out.append("skinparam backgroundColor #FFD1DC")
out.append("skinparam packageBackgroundColor #FFD1DC")
out.append("skinparam rectangle<<stub>> {")
out.append("  BackgroundColor transparent")
out.append("  BorderColor transparent")
out.append("  FontColor transparent")
out.append("}")
out.append("")

for g in TOP_ORDER:
    render_group((g,), 0)
    lines.append("")

out.extend(lines)

out.append("' -- wires --")
for w in wires:
    frm = w["From"].strip()
    to = w["To"].strip()
    if frm == "N/A" or to == "N/A":
        continue
    a_from = resolve_endpoint(frm)
    a_to = resolve_endpoint(to)
    # Keep the visual flow monotonic Topside -> External -> Main Housing
    # (top to bottom) regardless of which end the CSV lists first, so the
    # elk layered algorithm stacks the top-level groups in the requested order.
    idx_from = TOP_INDEX[base_alias(a_from)]
    idx_to = TOP_INDEX[base_alias(a_to)]
    if idx_from > idx_to:
        a_from, a_to = a_to, a_from
    style = style_for(w["Wire Color"], w["Wire Stripe Color"])
    label = w["Wire Label"].strip()
    line = f"{a_from} {style} {a_to}"
    if label:
        line += f" : {label}"
    out.append(line)

out.append("")
out.append("' -- unused/stub pin wires --")
for source_alias, target_alias, style, label in stub_edges:
    line = f"{source_alias} {style} {target_alias}"
    if label:
        line += f" : {label}"
    out.append(line)

out.append("")
out.append("@enduml")

result = "\n".join(out)
with open(f"{BASE}/plantuml.txt", "w", encoding="utf-8") as f:
    f.write(result)

print("Wrote plantuml.txt", file=sys.stderr)
print(f"Stub wires added: {len(stub_edges)}", file=sys.stderr)
