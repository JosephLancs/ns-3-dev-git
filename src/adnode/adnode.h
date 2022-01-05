#ifndef IPV4_ROUTING_HELPER_H
#define IPV4_ROUTING_HELPER_H

#include "ns3/application.h"
#include "ns3/wifi-phy.h"

namespace ns3
{
    class Adnode : public ns3::Application
    {
        public:
            static TypeId GetTypeId(void);
            virtual TypeId GetInstanceTypeId(void) const;

            Adnode();
            ~Adnode();

            void StartApplication();

            void BroadcastInformation();

            bool ReceivePacket(Ptr<NetDevice> device, Ptr<const Packet)> packet, uint16_t protocol, const Address &sender);

            void PromiscRx(Ptr<const Packet> packet, uint16_t channelFreq, WifiTxVector tx, MpduInfo mpdu, SignalNoiseDbm sn);

            


    }
}