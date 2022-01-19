#ifndef ADNODE_H
#define ADNODE_H

#include "ns3/application.h"
#include "ns3/wifi-phy.h"
#include "ns3/packet.h"
#include "ns3/ipv4-address.h"
#include "ns3/simulator.h"
#include "ns3/node-container.h"
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


        private:
            virtual void StartApplication(void);
            
            void Move(void);

            bool ReceivePacket(Ptr<NetDevice> device,
                               Ptr<const Packet> packet,
                               uint16_t protocol,
                               const Address &from,
                               const Address &to,
                               NetDevice::PacketType packetType);


    };
}

#endif