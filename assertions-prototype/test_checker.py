import sys

in1 = sys.argv[1]
in2 = sys.argv[2]

with open(in1, 'r') as f1:
    text1 = f1.read()
with open(in2, 'r') as f2:
    text2 = f2.read()

sat1 = '[sat]' in text1
sat2 = '[sat]' in text2

if sat1 != sat2:
    print('Output mismatch:')
    print(in1, ':', '[sat]' if sat1 else '[unsat]')
    print('vs')
    print(in2, ':', '[sat]' if sat2 else '[unsat]')
    exit(1)

exit(0)
