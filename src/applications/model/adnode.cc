#include "ns3/mobility-model.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/adversary-mobility-model.h"
#include "ns3/vector.h"
#include "ns3/node.h"
#include "ns3/adnode.h"
#include "ns3/uinteger.h"
#include "ns3/udp-header.h"
#include "ns3/packet.h"
#include "ns3/ipv4-header.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("Adnode");
    NS_OBJECT_ENSURE_REGISTERED(Adnode);

    TypeId Adnode::GetTypeId()
    {
        static TypeId tid = TypeId("ns3::Adnode")
            .SetParent<Application>()
            .AddConstructor<Adnode>()
            .AddAttribute ("Velocity", "The velocity of the adversary",
                   UintegerValue (1),
                   MakeUintegerAccessor (&Adnode::m_velocity),
                   MakeUintegerChecker<uint32_t> ())
            .AddAttribute ("DistanceToCalculate", "Distance that if the adversary if from the node the simulation will stop",
                    UintegerValue (5),
                    MakeUintegerAccessor (&Adnode::m_dist_to_calc),
                    MakeUintegerChecker<uint32_t> ())
            ;

        return tid;
    }

    TypeId Adnode::GetInstanceTypeId() const
    {
        return Adnode::GetTypeId();
    }

    Adnode::Adnode()
        : m_dpd (Seconds(60))
    {
        NS_LOG_FUNCTION (this);
    }

    void Adnode::Add_Sim_Source(Ptr<Node> src)
    {
        m_source_nodes.Add(src);
    }

    bool Adnode::HasReachedSource(Ptr<MobilityModel> me)
    {
        Ptr<MobilityModel> mob = GetNode()->GetObject<MobilityModel>();
        const Vector adpos = mob->GetPosition();
        //Vector srcpos;
        for(NodeContainer::Iterator n = m_source_nodes.Begin (); n != m_source_nodes.End (); n++)
        {
            Ptr<Node> object = *n;
            const Vector srcpos = object->GetObject<MobilityModel>()->GetPosition();
            if (CalculateDistance(adpos, srcpos) <= m_dist_to_calc)
            {
                NS_LOG_INFO("Adpos: " << adpos << " srcpos: " << srcpos);
                //fprintf(stdout, "Distance between stc%f\n", CalculateDistance(adpos, srcpos));
                return true;
            }
            else
            {
                NS_LOG_DEBUG("test");
                //fprintf(stdout, "Distance between stc%f\n", CalculateDistance(adpos, srcpos));
            }
        }
        return false;
    }
    
    bool Adnode::ReceivePacket(Ptr<NetDevice> device,
                               Ptr<const Packet> packet,
                               uint16_t protocol,
                               const Address &from,
                               const Address &to,
                               NetDevice::PacketType packetType)
    {
        NS_LOG_FUNCTION(this << "Receive adnode from:" << from << " to:" << to);

        Ptr<Packet> m_packet;
        Ipv4Header ipHeader;
        UdpHeader udpHeader;

        m_packet = packet -> Copy();
        m_packet->RemoveHeader (ipHeader);
        m_packet->PeekHeader(udpHeader);

        // Only process data packets
        uint16_t port = udpHeader.GetDestinationPort();
        NS_LOG_INFO("header" << port << ".");
        NS_LOG_DEBUG("ipheader" << ipHeader << ".");
        NS_LOG_DEBUG("udpheader" << udpHeader << ".");

        if (udpHeader.GetDestinationPort () != 9)
        {
            NS_LOG_INFO("adversary discarding packet - not on port 9");
            // AODV packets sent in broadcast are already managed - adversary discard
            return true;
        }

        // Check to see if the packet is a duplicate
        if (m_dpd.IsDuplicate(packet, ipHeader))
        {
            NS_LOG_INFO("Dropping duplicate packet " << packet << " header:" << ipHeader);
            return true;
        }

        NS_LOG_DEBUG("adversary processing packet");

        Ptr<MobilityModel> mob = GetNode()->GetObject<MobilityModel>();
        NS_ASSERT(mob);

        Ptr<AdversaryMobilityModel> admob = DynamicCast<AdversaryMobilityModel>(mob);
        NS_ASSERT(admob);
        //error logging for null node from address
        Ptr<Node> n = Adnode::GetNodeFromAddress(from);
        NS_LOG_FUNCTION("adnode from: " << from);
        NS_ASSERT(n);
        Ptr<MobilityModel> mobAdhoc = n->GetObject<MobilityModel>();
        
        NS_ASSERT(mobAdhoc);
        NS_LOG_DEBUG("adhoc: " << mobAdhoc->GetPosition());
        NS_LOG_DEBUG("mob: " << admob << admob->GetPosition());

        const Vector my_position = admob->GetPosition();
        Vector target_position = mobAdhoc->GetPosition();

        double distance = CalculateDistance(target_position, my_position);

        Time time_to_reach = Simulator::Now() + Seconds (distance / m_velocity);
  
        admob->SetTarget(time_to_reach, target_position);

        NS_LOG_DEBUG("Timetoreach: " << time_to_reach << " target pos: " << target_position);

        if(HasReachedSource(admob))
        {
            NS_LOG_INFO("Adversary has captured source node.");
            Simulator::Stop();
        }

        return true;
    }

    Ptr<Node> Adnode::GetNodeFromAddress(const Address &from)
    {
        NodeContainer nodes = NodeContainer::GetGlobal();
        //interate over all nodes, then interate over all netdevices the nodes have check if that netdevice has this address
        for(NodeContainer::Iterator n = nodes.Begin (); n != nodes.End (); n++)
        {
            Ptr<Node> object = *n;
            uint32_t num = object->GetNDevices();
            for(uint32_t i=0; i<num; i++)
            {
                if(object->GetDevice(i)->GetAddress() == from)
                {
                    return object;
                }
            }
        } 
        
        NS_LOG_ERROR("Failed to find node for " << from);
        return NULL;
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
    }
}
