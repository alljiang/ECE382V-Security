import random
import time
import re
import subprocess
import csv

patterns = [
    r'[\s\S]*NetRange:\s*(.*)\s*CIDR:[\s\S]*OrgName:\s*(.*)\s*OrgId:[\s\S]*',
    r'[\s\S]*inetnum:\s*(.*)\n[\s\S]*org-name:\s*(.*)\n*',
    r'[\s\S]*inetnum:\s*(.*)\n[\s\S]*descr:\s*(.*)\n*',
    r'[\s\S]*\[Network Number\]\s*(.*)\s*b. \[Network Name\][\s\S]*\[Organization\]\s*(.*)\s*m. \[Administrative Contact\][\s\S]*',
    r'[\s\S]*inetnum:\s*(.*)\n[\s\S]*owner:\s*(.*)\n*',
    r'[\s\S]*inetnum:\s*(.*)\n[\s\S]*netname:\s*(.*)\n*',
    r'[\s\S]*IPv4 Address       :\s*(.*)\n[\s\S]*Organization Name  :\s*(.*)\n*'
]

# open csv file
with open('/mnt/d/github/ECE382V-Security/Lab 1/Part 2/zmap_results.csv', 'r') as csvfile:
    # read csv file
    reader = csv.reader(csvfile, delimiter=',')

    read_all = []
    for row in reader:
        read_all.append(row)
    random.shuffle(read_all)

    address_map = {}

    count = 0

    for row in read_all:
        # get ip address from csv file
        ip = row[0]
        # print(ip)

        # get output of whois
        whois = subprocess.Popen(['timeout', '1', 'whois', ip], stdout=subprocess.PIPE)

        try:
            whois_out = whois.communicate()[0].decode()
        except:
            continue 

        for regex_pattern in patterns:
            regex_match = re.match(regex_pattern, whois_out)

            if regex_match:
                break

        try:
            net_range = regex_match.group(1)
            org_name = regex_match.group(2)
        except:
            print(whois_out)
        
        if not net_range in address_map:
            address_map[net_range] = (org_name, 1)
        else:
            address_map[net_range] = (org_name, address_map[net_range][1] + 1)

        count += 1
        percentage = count * 100 / 53598
        print("{:d}/53598 {:0.2f}%".format(count, percentage))        

csv_out = open('/mnt/d/github/ECE382V-Security/Lab 1/Part 2/whois_results.csv', 'w')
writer = csv.writer(csv_out)
writer.writerow(['Net Range', 'Organization Name', 'Machines Responding'])

for key in address_map:
    print(key, address_map[key])
    writer.writerow([key, address_map[key][0], address_map[key][1]])

csv_out.close()
