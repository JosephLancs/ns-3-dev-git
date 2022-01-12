#include "ns3/mobility-model.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
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

    /*bool Adnode::ReceiveCallback(Ptr<NetDevice> netdev,
                                 Ptr<const Packet> p,
                                 uint16_t,
                                 const Address &,
                                 const Address &,
                                 enum PacketType)
    {
        NS_LOG_FUNCTION (this);

        //TODO: print this out
    }*/

    void Adnode::StartApplication()
    {
        NS_LOG_FUNCTION (this);

        //for each netdevice
        // netdevice->SetPromiscReceiveCallback(PromiscReceiveCallback(Adnode::ReceiveCalback));
    }
}
