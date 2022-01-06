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
 *      NS-2 flooding model developed by the CMU/MONARCH group and optimized and
 *      tuned by Samir Das and Mahesh Marina, University of Cincinnati;
 *
 *      flooding-UU implementation by Erik Nordström of Uppsala University
 *      http://core.it.uu.se/core/index.php/flooding-UU
 *
 * Authors: Elena Buchatskaia <borovkovaes@iitp.ru>
 *          Pavel Boyko <boyko@iitp.ru>
 */
#ifndef FLOODINGROUTINGPROTOCOL_H
#define FLOODINGROUTINGPROTOCOL_H

#include "flooding-packet.h"
#include "flooding-dpd.h"
#include "ns3/timer.h"
#include "ns3/node.h"
#include "ns3/random-variable-stream.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include <map>

namespace ns3 {
namespace flooding {
/**
 * \ingroup flooding
 *
 * \brief flooding routing protocol
 */
class RoutingProtocol : public Ipv4RoutingProtocol
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /// constructor
  RoutingProtocol ();
  virtual ~RoutingProtocol ();
  virtual void DoDispose ();

  // Inherited from Ipv4RoutingProtocol
  Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p,
                              const Ipv4Header &header,
                              Ptr<NetDevice> oif,
                              Socket::SocketErrno &sockerr);
  
  bool RouteInput (Ptr<const Packet> p,
                   const Ipv4Header &header,
                   Ptr<const NetDevice> idev,
                   UnicastForwardCallback ucb,
                   MulticastForwardCallback mcb,
                   LocalDeliverCallback lcb,
                   ErrorCallback ecb);
  
  virtual void NotifyInterfaceUp (uint32_t interface);
  virtual void NotifyInterfaceDown (uint32_t interface);
  virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address);
  
  virtual void SetIpv4 (Ptr<Ipv4> ipv4);
  
  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const;

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);

protected:
  virtual void DoInitialize (void);

private:
  // Protocol parameters.
  //\}

  /// IP protocol
  Ptr<Ipv4> m_ipv4;
  /// 
  std::map<uint32_t, Ipv4InterfaceAddress> m_addresses;
  /// Loopback device used to defer RREQ until packet will be fully formed
  Ptr<NetDevice> m_lo;

  /// Handle duplicated broadcast/multicast packets
  DuplicatePacketDetection m_dpd;

private:
  /// Start protocol operation
  void Start ();

  /**
   * Test whether the provided address is assigned to an interface on this node
   * \param src the source IP address
   * \returns true if the IP address is the node's IP address
   */
  bool IsMyOwnAddress (Ipv4Address src) const;

  Ipv4Address
  GetRouteSource (const Ipv4Header & hdr, Ptr<NetDevice> oif) const;

  /**
   * Create loopback route for given header
   *
   * \param header the IP header
   * \param oif the output interface net device
   * \returns the route
   */
  Ptr<Ipv4Route> LoopbackRoute (const Ipv4Header & header, Ptr<NetDevice> oif) const;
  /**
   * Create broadcast route for given header
   *
   * \param header the IP header
   * \param oif the output interface net device
   * \returns the route
   */
  Ptr<Ipv4Route> BroadcastRoute (const Ipv4Header & header, Ptr<NetDevice> oif) const;

  Ptr<NetDevice> GetOutputNetDevice () const;

  /// Provides uniform random variables.
  Ptr<UniformRandomVariable> m_uniformRandomVariable;
};

} //namespace flooding
} //namespace ns3

#endif /* floodingROUTINGPROTOCOL_H */
