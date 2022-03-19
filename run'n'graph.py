import os
import dpkt
import matplotlib.pylab as plt

##########################
####### Parameters #######
##########################

protocol = 4
nodes = 100
advNodes = 2
simTime = 200
sendTime = 10
transmissionPower = -4
mobilityMod = 2 # 1. stationary; 2. mobile
deltax = 25
deltay = 25
seed = 50
nodeSpeed = 4

repeats = 1

resultsToPlot = {}
secondaryRTP = {}


def parseData(numNodes):
    advTotPackets = 0

    i=numNodes
    for i in range(numNodes,numNodes + advNodes):
        fn = "advNode-%s-0.pcap" % i
        counter = 0
        ttc = 0
        for ts, pkt in dpkt.pcap.Reader(open(fn,'rb')):
            counter+=1
            udp = dpkt.udp.UDP(pkt)
            ttc=ts       
        advTotPackets += counter
        print("Total number of packets in the adversary " + str(i) + " pcap file: " + str(counter))

    return ttc
    #resultsToPlot[numNodes] = ttc

def plotData():
    print("plotting data....")
    lst = sorted(resultsToPlot.items())
    x, y = zip(*lst)
    plt.title('Number of adhoc nodes vs time to capture in Flooding', size=16)
    plt.xlabel('Number of nodes', size=14)
    plt.ylabel('Time to capture', size=14)
    plt.plot(x, y)
    plt.show()


def main():
    global seed
    for numNodes in range(50, 250, 50):
        ttcs = 0
        for i in range(0,repeats):
            seed = seed + 1

            cmd = "./waf --run \"slp-manet-routing-compare --protocol=" + str(protocol) + \
                                                        " --nodes=" + str(numNodes) + \
                                                        " --adversary-nodes=" + str(advNodes) + \
                                                        " --total-time=" + str(simTime) + \
                                                        " --send-start=" + str(sendTime) + \
                                                        " --transmit-power=" + str(transmissionPower) + \
                                                        " --deltax=" + str(deltax) + \
                                                        " --deltay=" + str(deltay) + \
                                                        " --seed=" + str(seed) + \
                                                        " --node-speed=" + str(nodeSpeed) + \
                                                        " --Mobility-Model=" + str(mobilityMod) + "\""

            print(cmd)

            os.system(cmd)

            ttc = parseData(numNodes)

            ttcs = ttcs + ttc

        resultsToPlot[numNodes] = ttcs / repeats

            

    #print(resultsToPlot)
    plotData()



if __name__ == "__main__":
    main()