/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Based on
 *      NS-2 FLOODING model developed by the CMU/MONARCH group and optimized and
 *      tuned by Samir Das and Mahesh Marina, University of Cincinnati;
 *
 *      FLOODING-UU implementation by Erik Nordström of Uppsala University
 *      http://core.it.uu.se/core/index.php/FLOODING-UU
 *
 * Authors: Elena Buchatskaia <borovkovaes@iitp.ru>
 *          Pavel Boyko <boyko@iitp.ru>
 */
#define NS_LOG_APPEND_CONTEXT                                   \
  if (m_ipv4) { std::clog << "[node " << m_ipv4->GetObject<Node> ()->GetId () << "] "; }

#include "flooding-routing-protocol.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/random-variable-stream.h"
#include "ns3/inet-socket-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/udp-header.h"
#include "ns3/wifi-net-device.h"
#include "ns3/adhoc-wifi-mac.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include <algorithm>
#include <limits>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("FloodingRoutingProtocol");

namespace flooding {
NS_OBJECT_ENSURE_REGISTERED (RoutingProtocol);

/// UDP Port for flooding control traffic
const uint32_t RoutingProtocol::FLOODING_PORT = 654;

#if 0
/**
* \ingroup FLOODING
* \brief Tag used by FLOODING implementation
*/
class DeferredRouteOutputTag : public Tag
{

public:
  /**
   * \brief Constructor
   * \param o the output interface
   */
  DeferredRouteOutputTag (int32_t o = -1) : Tag (),
                                            m_oif (o)
  {
  }

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::flooding::DeferredRouteOutputTag")
      .SetParent<Tag> ()
      .SetGroupName ("Flooding")
      .AddConstructor<DeferredRouteOutputTag> ()
    ;
    return tid;
  }

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  TypeId  GetInstanceTypeId () const
  {
    return GetTypeId ();
  }
  
  /**
   * \brief Set the output interface
   * \param oif the output interface
   */
  void SetInterface (int32_t oif)
  {
    m_oif = oif;
  }

  uint32_t GetSerializedSize () const
  {
    return sizeof(int32_t);
  }

  void  Serialize (TagBuffer i) const
  {
    i.WriteU32 (m_oif);
  }

  void  Deserialize (TagBuffer i)
  {
    m_oif = i.ReadU32 ();
  }

  void  Print (std::ostream &os) const
  {
    os << "DeferredRouteOutputTag: output interface = " << m_oif;
  }

private:
  /// Positive if output device is fixed in RouteOutput
  int32_t m_oif;
};

NS_OBJECT_ENSURE_REGISTERED (DeferredRouteOutputTag);
#endif

void
RoutingProtocol::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
  *stream->GetStream () << "Node: " << m_ipv4->GetObject<Node> ()->GetId ()
                        << "; Time: " << Now ().As (unit)
                        << ", Local time: " << GetObject<Node> ()->GetLocalTime ().As (unit)
                        << std::endl;
}
//-----------------------------------------------------------------------------
RoutingProtocol::RoutingProtocol ()
  : m_dpd (Minutes (10))
{
}

TypeId
RoutingProtocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::flooding::RoutingProtocol")
    .SetParent<Ipv4RoutingProtocol> ()
    .SetGroupName ("Flooding")
    .AddConstructor<RoutingProtocol> ();

  return tid;
}

RoutingProtocol::~RoutingProtocol ()
{
}

void
RoutingProtocol::DoDispose ()
 {
   m_ipv4 = 0;
   m_addresses.clear ();
   Ipv4RoutingProtocol::DoDispose ();
 }

void
RoutingProtocol::Start ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<Ipv4Route>
RoutingProtocol::RouteOutput (Ptr<Packet> p, const Ipv4Header &header,
                              Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
  NS_LOG_FUNCTION (this << header << (oif ? oif->GetIfIndex () : 0));

  // Query routing cache for an existing route, for an outbound packet
  // It does not cause any packet to be forwarded, and is synchronous.
  // We should only output a route that says to send to the multicast address

  // No packet, so sort this out later
  if (!p)
  {
    NS_LOG_DEBUG ("Packet is == 0");
    return LoopbackRoute (header, oif); // later
  }

  /*if (m_socketAddresses.empty ())
  {
    NS_LOG_LOGIC ("No flooding interfaces");
    sockerr = Socket::ERROR_NOROUTETOHOST;
    Ptr<Ipv4Route> ipv4Route;
    return ipv4Route;
  }*/

  sockerr = Socket::ERROR_NOTERROR;

  if (oif)
  {
    NS_LOG_DEBUG ("Using broadcast route");
    return BroadcastRoute (header, oif);
  }
  else
  {
    oif = GetOutputNetDevice();
    NS_LOG_DEBUG ("Using broadcast route with "
      << m_ipv4->GetAddress(m_ipv4->GetInterfaceForDevice(oif), 0));
    return BroadcastRoute (header, oif);
  }

#if 0
  // Valid route not found, in this case we return loopback.
  // Actual route request will be deferred until packet will be fully formed,
  // routed to loopback, received from loopback and passed to RouteInput (see below)
  NS_LOG_DEBUG ("No output interface yet, deferring route");

  uint32_t iif = (oif ? m_ipv4->GetInterfaceForDevice (oif) : -1);
  DeferredRouteOutputTag tag (iif);
  if (!p->PeekPacketTag (tag))
    {
      p->AddPacketTag (tag);
    }
  return LoopbackRoute (header, oif);
#endif
}

#if 0
void
RoutingProtocol::DeferredRouteOutput (Ptr<const Packet> p,
                                      const Ipv4Header &header,
                                      UnicastForwardCallback ucb,
                                      MulticastForwardCallback mcb,
                                      ErrorCallback ecb)
{
  NS_LOG_FUNCTION (this << p << header);
  NS_ASSERT (p != 0 && p != Ptr<Packet> ());

  Ptr<NetDevice> oif = GetOutputNetDevice ();
  Ptr<Ipv4Route> ipv4Route = BroadcastRoute (header, oif);

  NS_LOG_DEBUG("DeferredRouteOutput to " << oif->GetIfIndex());

  ucb (ipv4Route, p, header);
}
#endif

bool
RoutingProtocol::RouteInput (Ptr<const Packet> p,
                             const Ipv4Header &header,
                             Ptr<const NetDevice> idev,
                             UnicastForwardCallback ucb,
                             MulticastForwardCallback mcb,
                             LocalDeliverCallback lcb,
                             ErrorCallback ecb)
{
  
  NS_LOG_FUNCTION (this << p->GetUid () << header.GetDestination () << idev->GetAddress ());
  
  if (m_addresses.empty ())
    {
      NS_LOG_LOGIC ("No flooding interfaces");
      return false;
    }

  NS_ASSERT (m_ipv4 != 0);
  NS_ASSERT (p != 0);

  // Check if input device supports IP
  NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);
  int32_t iif = m_ipv4->GetInterfaceForDevice (idev);

  Ipv4Address dst = header.GetDestination ();
  Ipv4Address origin = header.GetSource ();

  // Deferred route request
  NS_ASSERT(idev != m_lo);
  /*if (idev == m_lo)
    {
      DeferredRouteOutputTag tag;
      if (p->PeekPacketTag (tag))
        {
          DeferredRouteOutput (p, header, ucb, mcb, ecb);
          return true;
        }
    }*/

  // Duplicate of own packet
  if (IsMyOwnAddress (origin))
    {
      NS_LOG_DEBUG ("Duplicate packet detected");
      return true;
    }

  // FLOODING is not a multicast routing protocol
  /*if (dst.IsMulticast ())
    {
      return false;
    }*/

#if 0
  // Broadcast local delivery/forwarding
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
         m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {
      Ipv4InterfaceAddress iface = j->second;
      if (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()) == iif)
        {
          if (dst == iface.GetBroadcast () || dst.IsBroadcast ())
            {
              if (m_dpd.IsDuplicate (p, header))
                {
                  NS_LOG_DEBUG ("Duplicated packet " << p->GetUid () << " from " << origin << ". Drop.");
                  return true;
                }

              Ptr<Packet> packet = p->Copy ();
              if (!lcb.IsNull ())
                {
                  NS_LOG_LOGIC ("Broadcast local delivery to " << iface.GetLocal ());
                  lcb (p, header, iif);
                  // Fall through to additional processing
                }
              else
                {
                  NS_LOG_ERROR ("Unable to deliver packet locally due to null callback " << p->GetUid () << " from " << origin);
                  ecb (p, header, Socket::ERROR_NOROUTETOHOST);
                }

              /*if (!m_enableBroadcast)
                {
                  return true;
                }*/

              if (header.GetProtocol () == UdpL4Protocol::PROT_NUMBER)
                {
                  UdpHeader udpHeader;
                  p->PeekHeader (udpHeader);
                  if (udpHeader.GetDestinationPort () == FLOODING_PORT)
                    {
                      // flooding packets sent in broadcast are already managed
                      return true;
                    }
                }

              if (header.GetTtl () > 1)
                {
                  NS_LOG_LOGIC ("Forward broadcast. TTL " << (uint16_t) header.GetTtl ());
 
                  Ptr<Ipv4Route> ipv4Route = BroadcastRoute (header, GetOutputNetDevice ());
                  ucb (ipv4Route, packet, header);
                }
              else
                {
                  NS_LOG_DEBUG ("TTL exceeded. Drop packet " << p->GetUid ());
                }
              return true;
            }
        }
    }
#endif

  // Unicast local delivery
  if (m_ipv4->IsDestinationAddress (dst, iif))
    {
      if (!lcb.IsNull ())
        {
          NS_LOG_LOGIC ("Unicast local delivery to " << dst);
          lcb (p, header, iif);
        }
      else
        {
          NS_LOG_ERROR ("Unable to deliver packet locally due to null callback " << p->GetUid () << " from " << origin);
          ecb (p, header, Socket::ERROR_NOROUTETOHOST);
        }
      return true;
    }

  // Check if input device supports IP forwarding
  if (!m_ipv4->IsForwarding (iif))
    {
      NS_LOG_LOGIC ("Forwarding disabled for this interface");
      ecb (p, header, Socket::ERROR_NOROUTETOHOST);
      return true;
    }

  // Forwarding
  Ptr<Ipv4Route> route = BroadcastRoute(header, GetOutputNetDevice ());
  ucb (route, p, header);

  return true;
}

void
RoutingProtocol::SetIpv4 (Ptr<Ipv4> ipv4)
{
  NS_ASSERT (ipv4 != 0);
  NS_ASSERT (m_ipv4 == 0);

  m_ipv4 = ipv4;

  // Create lo route. It is asserted that the only one interface up for now is loopback
  NS_ASSERT (m_ipv4->GetNInterfaces () == 1 && m_ipv4->GetAddress (0, 0).GetLocal () == Ipv4Address::GetLoopback ());
  m_lo = m_ipv4->GetNetDevice (0);
  NS_ASSERT (m_lo != 0);

  Simulator::ScheduleNow (&RoutingProtocol::Start, this);
}

void
RoutingProtocol::NotifyInterfaceUp (uint32_t i)
{
  NS_LOG_FUNCTION (this << i << m_ipv4->GetAddress (i, 0).GetLocal ());
  
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
  if (l3->GetNAddresses (i) > 1)
    {
      NS_LOG_WARN ("flooding does not work with more then one address per each interface.");
    }
  Ipv4InterfaceAddress iface = l3->GetAddress (i, 0);
  if (iface.GetLocal () == Ipv4Address::GetLoopback ())
    {
      return;
    }

  m_addresses.insert (std::make_pair (i, iface));
}

void
RoutingProtocol::NotifyInterfaceDown (uint32_t i)
{
  NS_LOG_FUNCTION (this << m_ipv4->GetAddress (i, 0).GetLocal ());

  m_addresses.erase (i);
}

void
RoutingProtocol::NotifyAddAddress (uint32_t i, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION (this << " interface " << i << " address " << address);
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
  if (!l3->IsUp (i))
    {
      return;
    }
  if (l3->GetNAddresses (i) == 1)
    {
      Ipv4InterfaceAddress iface = l3->GetAddress (i, 0);
      if (iface.GetLocal () == Ipv4Address::GetLoopback ())
        {
          return;
        }

      m_addresses.insert (std::make_pair (i, iface));
    }
  else
    {
      NS_LOG_LOGIC ("Flooding does not work with more then one address per each interface. Ignore added address");
    }
}

void
RoutingProtocol::NotifyRemoveAddress (uint32_t i, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION (this);
  m_addresses.erase (i);
}

bool
RoutingProtocol::IsMyOwnAddress (Ipv4Address src) const
{
  NS_LOG_FUNCTION (this << src);
  for (auto j = m_addresses.cbegin (); j != m_addresses.cend (); ++j)
    {
      Ipv4InterfaceAddress iface = j->second;
      if (src == iface.GetLocal ())
        {
          return true;
        }
    }
  return false;
}

int64_t
RoutingProtocol::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uniformRandomVariable->SetStream (stream);
  return 1;
}

void
RoutingProtocol::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  Ipv4RoutingProtocol::DoInitialize ();
}

Ipv4Address
RoutingProtocol::GetRouteSource (const Ipv4Header & hdr, Ptr<NetDevice> oif) const
{
  NS_LOG_FUNCTION (this << hdr);

  //
  // Source address selection here is tricky.  The loopback route is
  // returned when AODV does not have a route; this causes the packet
  // to be looped back and handled (cached) in RouteInput() method
  // while a route is found. However, connection-oriented protocols
  // like TCP need to create an endpoint four-tuple (src, src port,
  // dst, dst port) and create a pseudo-header for checksumming.  So,
  // AODV needs to guess correctly what the eventual source address
  // will be.
  //
  // For single interface, single address nodes, this is not a problem.
  // When there are possibly multiple outgoing interfaces, the policy
  // implemented here is to pick the first available AODV interface.
  // If RouteOutput() caller specified an outgoing interface, that
  // further constrains the selection of source address
  //
  auto j = m_addresses.cbegin ();
  if (oif)
    {
      // Iterate to find an address on the oif device
      for (j = m_addresses.cbegin (); j != m_addresses.cend (); ++j)
        {
          Ipv4Address addr = j->second.GetLocal ();
          int32_t interface = m_ipv4->GetInterfaceForAddress (addr);

          if (interface < 0)
          {
            continue;
          }

          uint32_t uinterface = static_cast<uint32_t>(interface);

          NS_ASSERT (j->first == uinterface);

          if (oif == m_ipv4->GetNetDevice (uinterface))
            {
              return addr;
            }
        }
    }

    NS_ASSERT (j != m_addresses.cend());

    return j->second.GetLocal ();
}

Ptr<Ipv4Route>
RoutingProtocol::LoopbackRoute (const Ipv4Header & hdr, Ptr<NetDevice> oif) const
{
  NS_LOG_FUNCTION (this << hdr);
  NS_ASSERT (m_lo != 0);

  Ptr<Ipv4Route> rt = Create<Ipv4Route> ();
  rt->SetDestination (hdr.GetDestination ());
  rt->SetSource (GetRouteSource(hdr, oif));
  rt->SetGateway (Ipv4Address::GetLoopback ());
  rt->SetOutputDevice (m_lo);

  NS_ASSERT_MSG (rt->GetSource () != Ipv4Address (), "Valid source address not found");

  return rt;
}

Ptr<Ipv4Route>
RoutingProtocol::BroadcastRoute (const Ipv4Header & hdr, Ptr<NetDevice> oif) const
{
  NS_LOG_FUNCTION (this << hdr);

  Ipv4Address src = hdr.GetSource ();
  if (src == Ipv4Address ())
  {
    src = GetRouteSource(hdr, oif);
  }

  Ptr<Ipv4Route> rt = Create<Ipv4Route> ();
  rt->SetDestination (hdr.GetDestination ());
  rt->SetSource (src);
  //rt->SetGateway (Ipv4Address::GetAny ());
  rt->SetGateway (Ipv4Address::GetBroadcast ());
  rt->SetOutputDevice (oif);

  NS_ASSERT_MSG (rt->GetSource () != Ipv4Address (), "Valid source address not found");

  return rt;
}

Ptr<NetDevice>
RoutingProtocol::GetOutputNetDevice () const
{
  NS_LOG_FUNCTION (this);

  // Find the only device other than the loopback which is up
  uint32_t ninterfaces = m_ipv4->GetNInterfaces();

  for (uint32_t i = 0; i != ninterfaces; ++i)
  {
    Ptr<NetDevice> netdev = m_ipv4->GetNetDevice(i);
    if (netdev != m_lo)
    {
      //NS_LOG_DEBUG("Found device " << i << "/" << ninterfaces
      //             << " with address " << m_ipv4->GetAddress(i, 0));
      return netdev;
    }
  }

  NS_LOG_ERROR("Failed to find output device");
  Ptr<NetDevice> netdev;
  return netdev;
}

} //namespace flooding
} //namespace ns3
