from core import *
import sys
from lootgen import LootGenSimulation


sim = LootGenSimulation()
sim.generate_test_case(seed=int(sys.argv[1]), print_constr=False, print_map=False)

consumed_attribute = {}
consumed_item = {}
for c in sim.constraints:
    key_item = c.signature.upto(1)
    if key_item not in consumed_item:
        consumed_item[key_item] = IntRange(0,0)

    if len(c.signature.words) == 1:
        continue
    key_attr = c.signature.upto(2)
    if key_attr not in consumed_attribute:
        consumed_attribute[key_attr] = IntRange(0,0)


# top-level constraints
for c in sim.constraints:
    if len(c.signature.words) != 3:
        continue
    n = sim.item_map[c.signature.tostring()]
    if not (c.count.min <= n <= c.count.max):
        unsat('bad top level constraint')

    consumed_attribute[c.signature.upto(2)] += IntRange(c.count.min, min(n, c.count.max))

# attr-level constraints
for c in sim.constraints:
    if len(c.signature.words) != 2:
        continue
    n = sim.item_map[c.signature.tostring()]
    cons = consumed_attribute[c.signature.tostring()]
    real_count = IntRange(n, n).consume(cons)

    if not real_count.intersects(c.count):
        unsat('bad attribute level constraint')

    # we consumed `cons` already at the attribute+level level,
    # and are left with `real_count` items. 
    # if the constraint is == then we're consuming exactly that many,
    # otherwise we're consuming the min count or anything above, as long
    # as it's below the number of total available items
    consumed: IntRange = cons + c.count
    consumed.max = min(n, c.count.max)
    consumed_item[c.signature.upto(1)] += consumed

# item-level constraints
for c in sim.constraints:
    if len(c.signature.words) != 1:
        continue
    n = sim.item_map[c.signature.tostring()]
    cons = consumed_item[c.signature.tostring()]
    real_count = IntRange(n, n).consume(cons)

    if not real_count.intersects(c.count) or real_count.consume(c.count).min != 0:
        unsat('bad item level constraint')

sat()