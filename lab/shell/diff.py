import re

tsh_trace = []
tshref_trace = []

for i in range(1, 17):
    tsh_trace.append('tsh_trace' + ('0' + str(i) if i < 10 else str(i)) + '.txt')
    tshref_trace.append('tshref_trace' + ('0' + str(i) if i < 10 else str(i)) + '.txt')

for f1, f2 in zip(tsh_trace, tshref_trace):
    f1 = open(f1, 'r')
    f2 = open(f2, 'r')
    lines1 = f1.readlines()
    lines2 = f2.readlines()
    ans1, ans2 = [], []
    for line in lines1:
        if line.startswith('tsh> '):
            ans1.append(line)
    for line in lines2:
        if line.startswith('tsh> '):
            ans2.append(line)
    assert len(ans1) == len(ans2)
    # print(ans1)
    # print(ans2)
    for line1, line2 in zip(ans1, ans2):
        line1 = re.sub(r'.*(\d+)', r'', line1.strip('\n').strip(' '))
        line2 = re.sub(r'.*(\d+)', r'', line2.strip('\n').strip(' '))
        # print('line1: ', line1)
        # print('line2: ', line2)
        assert line1 == line2
    f1.close()
    f2.close()

print('passed!!!!')
