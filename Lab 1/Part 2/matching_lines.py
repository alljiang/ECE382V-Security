# read from set1.txt
set1 = []
set2 = []

with open('/mnt/d/github/ECE382V-Security/Lab 1/Part 2/set1.txt', 'r') as set1file:
    for line in set1file:
        set1.append(line.strip())

with open('/mnt/d/github/ECE382V-Security/Lab 1/Part 2/set2.txt', 'r') as set2file:
    for line in set2file:
        set2.append(line.strip())

# find matching lines between set1 and set2
matching_lines = []
for line in set1:
    if line in set2:
        matching_lines.append(line)

print(matching_lines)