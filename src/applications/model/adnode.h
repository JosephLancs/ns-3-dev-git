#ifndef ADNODE_H
#define ADNODE_H

#include "ns3/application.h"
#include "ns3/wifi-phy.h"
#include "ns3/packet.h"
#include "ns3/ipv4-address.h"
#include "ns3/simulator.h"
#include <vector>



namespace ns3
{
    class Adnode : public ns3::Application
    {
        public:
            static TypeId GetTypeId(void);
            virtual TypeId GetInstanceTypeId(void) const;

            Adnode();
            //~Adnode();


            void BroadcastInformation(void);

            bool ReceivePacket(Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &sender);

            void PromiscRx(Ptr<const Packet> p, uint16_t channelFreq, WifiTxVector tx, MpduInfo mpdu, SignalNoiseDbm sn);

            void SetBroadcastInterval(Time interval);

        private:
            virtual void StartApplication(void);
            Time m_broadcast_time;
            uint32_t m_packetSize;

            //Ptr<NetDevice> m_netDevice;

            Time m_time_limit; //Time limit to keep neighbours in a list

            //WifiMode m_mode;


    };
}

#endif