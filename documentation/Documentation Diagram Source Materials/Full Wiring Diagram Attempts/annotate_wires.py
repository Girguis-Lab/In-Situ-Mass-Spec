import csv

edits = {
    2: {'to': 9},
    3: {'to': 10},
    4: {'to': 11},
    5: {'to': 12},
    6: {'from': 9, 'to': 1},
    7: {'from': 10, 'to': 2},
    8: {'from': 11, 'to': 3},
    9: {'from': 12, 'to': 5},
    10: {'to': 4},
    11: {'to': 4},
    12: {'to': 5},
    13: {'to': 8},
    14: {'from': 4, 'to': 4},
    15: {'from': 5, 'to': 5},
    16: {'from': 8, 'to': 8},
    17: {'from': 4},
    18: {'from': 5},
    19: {'from': 8},
    25: {'to': 14},
    26: {},
    27: {'to': 13},
    28: {'to': 15},
    29: {'from': 13, 'to': 1},
    30: {'from': 14, 'to': 2},
    31: {'from': 15, 'to': 3},
    32: {'from': 16, 'to': 4},
    33: {'to': 5},
    34: {'to': 6},
    35: {'to': 1},
    36: {'to': 2},
    37: {'to': 3},
    38: {'from': 1, 'to': 1},
    39: {'from': 2, 'to': 2},
    40: {'from': 3, 'to': 3},
    41: {'from': 1},
    42: {'from': 2},
    43: {'from': 3},
    51: {'to': 6},
    52: {'to': 7},
    56: {'from': 6, 'to': 6},
    57: {'from': 7, 'to': 7},
    58: {'from': 6},
    59: {'from': 7},
}

with open('wire_links.csv', newline='') as f:
    rows = list(csv.reader(f))

out = [rows[0]]
for i, row in enumerate(rows[1:], start=2):
    frm, to, color, stripe, label = row
    e = edits.get(i, {})
    if 'from' in e:
        frm = f"{frm} [{e['from']}]"
    if 'to' in e:
        to = f"{to} [{e['to']}]"
    out.append([frm, to, color, stripe, label])

with open('wire_links_with_pins.csv', 'w', newline='') as f:
    csv.writer(f).writerows(out)

print("done")
