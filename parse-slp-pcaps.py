import dpkt

counter=0
ipcounter=0
tcpcounter=0
udpcounter=0

adhocNodes=100
adversaryNodes=3

filename='adhocNode-69-0.pcap'
#########adhoc
i=1
for i in range(0,adhocNodes):
    fn = "adhocNode-%s-0.pcap" % i
    counter = 0
    for ts, pkt in dpkt.pcap.Reader(open(fn,'rb')):
        counter+=1
        udp = dpkt.udp.UDP(pkt)
        #print(udp.dport)
        #print(udp.sport)
        #print(udp.ulen)
        #print(udp.sum)    

    print("Total number of packets in the adhoc " + str(i) + " pcap file: " + str(counter))

i=adhocNodes
for i in range(adhocNodes,adhocNodes + adversaryNodes):
    fn = "advNode-%s-0.pcap" % i
    counter = 0
    for ts, pkt in dpkt.pcap.Reader(open(fn,'rb')):
        counter+=1
        udp = dpkt.udp.UDP(pkt)
        #print(udp.dport)
        #print(udp.sport)
        #print(udp.ulen)
        #print(udp.sum)        

    print("Total number of packets in the adversary " + str(i) + " pcap file: " + str(counter))

#for ts, pkt in dpkt.pcap.Reader(open(filename,'rb')):
#    print("pkt:", pkt)
#    print("ts: ", ts)
#    counter+=1
#    eth=dpkt.ethernet.Ethernet(pkt)
#    print("eth: ", eth.type)
#    if eth.type!=dpkt.ethernet.ETH_TYPE_IP:
#       continue#=
#
#    ip=eth.data
#    ipcounter+=1#
#
#    if ip.p==dpkt.ip.IP_PROTO_TCP: 
#       tcpcounter+=1##
#
#    if ip.p==dpkt.ip.IP_PROTO_UDP:
#       udpcounter+=1
