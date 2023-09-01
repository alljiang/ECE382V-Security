import pcapy

# returns average_packet_size, num_packets
def analyze_pcap(pcap_file_path) -> (int, int):
    reader = pcapy.open_offline(pcap_file_path)
    total_size = 0
    num_packets = 0
    while True:
        packet = reader.next()
        if not packet or len(packet[1]) == 0:
            break
        
        total_size += len(packet[1])
        num_packets += 1

    return total_size / num_packets, num_packets

prefix = '/mnt/d/Github/ECE382V-Security/Lab 1/Part 3/'
folders = ['firefox', 'vpn', 'tor']
web_names = ['wiki-cat', 'wiki-dog', 'wiki-egress', 'mit', 'unm', 'cmu', 'berkeley', 'utexas', 'asu', 'utdallas']

for folder in folders:
    browser_packet_sizes = []
    browser_num_packets = []
    for i in range(0, 10):
        folder_path = prefix + folder + '/' + str(i) + '/'
        website_name = web_names[i]

        packet_sizes_list = []
        num_packets_list = []

        for iteration in range(0, 10):
            pcap_file_path = folder_path + str(iteration) + '.pcap'
            avg_packet_size, num_packets = analyze_pcap(pcap_file_path)
            packet_sizes_list.append(int(avg_packet_size))
            num_packets_list.append(num_packets)
        
        # find median of packet sizes and num packets
        packet_sizes_list.sort()
        num_packets_list.sort()
        median_packet_size = int(packet_sizes_list[5])
        median_num_packets = num_packets_list[5]
        
        print(folder + ' ' + website_name + ' ' + str(median_packet_size) + ' ' + str(median_num_packets))
        browser_packet_sizes.append(packet_sizes_list)
        browser_num_packets.append(num_packets_list)
    
    print(folder + '_packet_sizes = [' + ''.join([str(x) + ',' for x in browser_packet_sizes]) + ']')
    print(folder + '_num_packets = [' + ''.join([str(x) + ',' for x in browser_num_packets]) + ']')
