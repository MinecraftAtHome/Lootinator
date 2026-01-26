from core import *
import sys
from lootgen import LootGenSimulation


sim = LootGenSimulation()
sim.generate_test_case(seed=int(sys.argv[1]), print_constr=True, print_map=True)

consumed_range_map: dict[str, IntRange] = {}
consumed_items = IntRange(0,0)
leftover = False
prev_signature = Signature.empty()

for constraint in sim.constraints:
    current_sign = constraint.signature
    common = current_sign.intersect(prev_signature)
    if not common.is_empty():
        # consume only if we're inside the same item group
        ckey = common.tostring()
        if ckey not in consumed_range_map:
            consumed_range_map[ckey] = IntRange(0,0)
        consumed_range_map[ckey] += consumed_items
    elif leftover:
        unsat('leftover items in loop')

    # calculate accumulator range
    key = current_sign.tostring()
    ab = IntRange(0,0) if key not in consumed_range_map else consumed_range_map[key]
    n_items = sim.item_map[key]

    # test constraint
    A = constraint.count.min
    N = n_items
    if constraint.count.max == INF:
        # greater equal case
        if N >= A + ab.min:
            pass
        else:
            unsat(f'GE assert failed, N={N}, A={A}, a={ab.min}, b={ab.max}')
        consumed_items = ab + IntRange(A, INF)
        consumed_items.max = n_items
    else:
        # equal case
        if A + ab.min <= N <= A + ab.max:
            pass
        else:
            unsat(f'EQ assert failed, N={N}, A={A}, a={ab.min}, b={ab.max}')
        consumed_items = ab + IntRange(A, A)

    prev_signature = current_sign
    leftover = IntRange(n_items, n_items).consume(consumed_items).min > 0

if leftover:
    unsat('leftover items after loop')
else:
    sat()
