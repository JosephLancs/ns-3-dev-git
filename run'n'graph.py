import os
import dpkt
import matplotlib.pylab as plt

##########################
####### Parameters #######
##########################

protocol = 4
nodes = 100
advNodes = 3
simTime = 200
sendTime = 10
transmissionPower = 7.5
mobilityMod = 1 # 1. stationary; 2. mobile

resultsToPlot = {}
secondaryRTP = {}


def parseData(numNodes):
    advTotPackets = 0

    i=numNodes
    for i in range(numNodes,numNodes + advNodes):
        fn = "advNode-%s-0.pcap" % i
        counter = 0
        for ts, pkt in dpkt.pcap.Reader(open(fn,'rb')):
            counter+=1
            udp = dpkt.udp.UDP(pkt)       
        advTotPackets += counter
        print("Total number of packets in the adversary " + str(i) + " pcap file: " + str(counter))

    resultsToPlot[numNodes] = advTotPackets

def plotData():
    print("plotting data....")
    lst = sorted(resultsToPlot.items())
    x, y = zip(*lst)
    plt.title('Number of adhoc nodes vs packets captured by adversary in Flooding', size=16)
    plt.xlabel('Number of nodes', size=14)
    plt.ylabel('Packets captured', size=14)
    plt.plot(x, y)
    plt.show()


def main():

    for numNodes in range(50, 150, 5):
        cmd = "./waf --run \"slp-manet-routing-compare --protocol=" + str(protocol) + \
                                                    " --nodes=" + str(numNodes) + \
                                                    " --adversary-nodes=" + str(advNodes) + \
                                                    " --total-time=" + str(simTime) + \
                                                    " --send-start=" + str(sendTime) + \
                                                    " --transmit-power=" + str(transmissionPower) + \
                                                    " --Mobility-Model=" + str(mobilityMod) + "\""

        print(cmd)

        os.system(cmd)

        parseData(numNodes)

    print(resultsToPlot)
    plotData()



if __name__ == "__main__":
    main()