from core import *
from random import randint, choice, seed as set_seed
from pprint import pprint


class LootGenSimulation:
    items = 2
    attributes = 2
    levels = 5
    max_per_roll = 10
    
    generated_items = 10
    num_constraints = 5
    item_map: dict[str, int] = {}
    constraints: list[Constraint] = []
    
    def update_item_map(self, signature: Signature, count: int):
        # update map at signature and all parent signatures
        for slen in range(1, len(signature.words) + 1):
            frag = signature.upto(slen)
            if frag not in self.item_map:
                self.item_map[frag] = 0
            self.item_map[frag] += count

    def generate_random_constraints(self):
        for _ in range(self.num_constraints):
            i = randint(1, self.items)
            a = randint(1, self.attributes)
            l = randint(1, self.levels)
            cnt_min = randint(1, self.max_per_roll)
            cnt_max = INF if randint(1, 2) == 1 else cnt_min
            slen = randint(1, 3)
            sign = Signature([i, a, l][:slen])
            fail = False
            for constraint in self.constraints:
                if constraint.signature.tostring() == sign.tostring():
                    fail = True
                    break
            if not fail: 
                self.constraints.append(Constraint(sign, IntRange(cnt_min, cnt_max)))

        self.constraints.sort()
        for constraint in self.constraints:
            self.update_item_map(constraint.signature, 0)

    def generate_random_loot(self):
        for _ in range(self.generated_items):
            i = randint(1, self.items)
            a = randint(1, self.attributes)
            l = randint(1, self.levels)
            cnt = randint(1, self.max_per_roll)
            sign = Signature([i, a, l])
            self.update_item_map(sign, cnt)

    def generate_test_case(self, seed=None, print_constr=False, print_map=False):
        self.item_map.clear()
        self.constraints.clear()

        set_seed(seed)
        self.generate_random_constraints()
        self.generate_random_loot()

        if print_constr:
            pprint(self.constraints)
        if print_map:
            pprint(self.item_map)


if __name__ == '__main__':
    sim = LootGenSimulation()
    sim.generate_test_case()
