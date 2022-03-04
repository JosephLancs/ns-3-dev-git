#ifndef ADNODE_H
#define ADNODE_H

#include "ns3/application.h"
#include "ns3/wifi-phy.h"
#include "ns3/packet.h"
#include "ns3/ipv4-address.h"
#include "ns3/simulator.h"
#include "ns3/node-container.h"
//#include "ns3/aodv-dpd.h"
#include <vector>



namespace ns3
{
    class Adnode : public ns3::Application
    {
        public:
            static TypeId GetTypeId(void);
            virtual TypeId GetInstanceTypeId(void) const;

            Adnode();
            virtual ~Adnode() = default;

            bool HasReachedSource(Ptr<MobilityModel> me);

            void Add_Sim_Source(Ptr<Node> src);


        private:
            virtual void StartApplication(void);

            Ptr<Node> GetNodeFromAddress(const Address &from);

            bool ReceivePacket(Ptr<NetDevice> device,
                               Ptr<const Packet> packet,
                               uint16_t protocol,
                               const Address &from,
                               const Address &to,
                               NetDevice::PacketType packetType);

        private:
            uint32_t m_velocity;
            NodeContainer m_source_nodes;
            //DuplicatePacketDetection m_dpd;
            //list(Ptr<Node>) m_source_nodes;
    };
}

#endif