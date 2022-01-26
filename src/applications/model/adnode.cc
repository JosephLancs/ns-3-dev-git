#include "ns3/mobility-model.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/adversary-mobility-model.h"
#include "ns3/vector.h"
#include "ns3/node.h"
#include "adnode.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("Adnode");
    NS_OBJECT_ENSURE_REGISTERED(Adnode);



    TypeId Adnode::GetTypeId()
    {
        static TypeId tid = TypeId("ns3::Adnode")
            .SetParent<Application>()
            .AddConstructor<Adnode>();

        return tid;
    }

    TypeId Adnode::GetInstanceTypeId() const
    {
        return Adnode::GetTypeId();
    }

    Adnode::Adnode()
    {
        NS_LOG_FUNCTION (this);
    }

    bool Adnode::ReceivePacket(Ptr<NetDevice> device,
                               Ptr<const Packet> packet,
                               uint16_t protocol,
                               const Address &from,
                               const Address &to,
                               NetDevice::PacketType packetType)
    {

        NS_LOG_FUNCTION(this << "Receive adnode: " << packet);

        //TODO: consider packet filtering

        Ptr<AdversaryMobilityModel> mob = GetObject<AdversaryMobilityModel>();

        //error logging for null node from address
        Ptr<Node> n = Adnode::GetNodeFromAddress(from);
        Ptr<MobilityModel> mobAdhoc = n->GetObject<MobilityModel>();

        Vector adPosition = mobAdhoc->GetPosition();

        bool hasAccept = mob->SetTarget(adPosition); //write this function
        
        if(hasAccept)
        {

        }
        return false;
    }

    Ptr<Node> Adnode::GetNodeFromAddress(const Address &from)
    {
        NodeContainer nodes = NodeContainer::GetGlobal();
        //interate over all nodes, then interate over all netdevices the nodes have check if that netdevice has this address
        for(NodeContainer::Iterator n = nodes.Begin (); n != nodes.End (); n++)
        {
            Ptr<Node> object = *n;
            if(object->GetDevice(1)->GetAddress() == from)
            {
                return object;
            }
        } 
        
        return NULL;

    }

    void Adnode::Move(void)
    {
        
    }


    void Adnode::StartApplication()
    {
        NS_LOG_FUNCTION (this);

        Ptr<Node> n = GetNode ();
        for (uint32_t i = 0; i < n->GetNDevices (); i++)
        {
            Ptr<NetDevice> dev = n->GetDevice (i);
            dev->SetPromiscReceiveCallback(MakeCallback(&Adnode::ReceivePacket, this));
        }

        Simulator::Schedule(Seconds(1), &Adnode::Move, this);
    }
}
