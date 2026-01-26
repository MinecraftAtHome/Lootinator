from dataclasses import dataclass
from pprint import pprint

INF = 100000

def unsat(msg):
    print('[unsat]:', msg)
    exit(0)

def sat():
    print('[sat]')
    exit(0)


@dataclass
class Signature:
    words: list[int]

    def print(self):
        print(self.words)

    def tostring(self) -> str:
        return self.upto(len(self.words))

    def upto(self, num_segments: int) -> str:
        return "_".join([str(word) for word in self.words[:min(len(self.words), num_segments)]])
    
    def is_empty(self) -> bool:
        return len(self.words) == 0

    def intersect(self, other: Signature) -> Signature:
        ml = min(len(self.words), len(other.words))
        sign = []
        for i in range(0, ml):
            if self.words[i] != other.words[i]:
                break
            sign.append(self.words[i])
        return Signature(sign)

    @staticmethod
    def empty() -> Signature:
        return Signature([])

    def __lt__(self, other: Signature):
        ml = min(len(self.words), len(other.words))
        for i in range(ml):
            if self.words[i] < other.words[i]:
                return True
            elif self.words[i] > other.words[i]:
                return False
        return len(self.words) > len(other.words)
    

@dataclass
class IntRange:
    min: int
    max: int

    def __add__(self, other: IntRange):
        return IntRange(self.min + other.min, self.max + other.max)
    
    def __sub__(self, other: IntRange):
        return IntRange(self.min - other.min, self.max - other.max)
    
    def consume(self, consumed: IntRange):
        min_leftover = max(0, self.min - consumed.max)
        max_leftover = self.max - consumed.min
        return IntRange(min_leftover, max_leftover)
    
    def intersects(self, other: IntRange) -> bool:
        return self.min <= other.max and other.min <= self.max


@dataclass
class Constraint:
    signature: Signature
    count: IntRange

    def __lt__(self, other: Constraint):
        return self.signature < other.signature



if __name__ == '__main__':
    sigs = [
        Signature([1, 2]),
        Signature([2, 2]),
        Signature([1]),
        Signature([1, 2, 1]),
        Signature([2]),
        Signature([1, 2, 3])
    ]
    #pprint(sigs)
    pprint(sorted(sigs))
