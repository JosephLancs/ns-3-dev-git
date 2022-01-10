/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 University of Kansas
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
 * Author: Justin Rohrer <rohrej@ittc.ku.edu>
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  http://wiki.ittc.ku.edu/resilinets
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */

/*
 * This example program allows one to run ns-3 DSDV, AODV, or OLSR under
 * a typical random waypoint mobility model.
 *
 * By default, the simulation runs for 200 simulated seconds, of which
 * the first 50 are used for start-up time.  The number of nodes is 50.
 * Nodes move according to RandomWaypointMobilityModel with a speed of
 * 20 m/s and no pause time within a 300x1500 m region.  The WiFi is
 * in ad hoc mode with a 2 Mb/s rate (802.11b) and a Friis loss model.
 * The transmit power is set to 7.5 dBm.
 *
 * It is possible to change the mobility and density of the network by
 * directly modifying the speed and the number of nodes.  It is also
 * possible to change the characteristics of the network by changing
 * the transmit power (as power increases, the impact of mobility
 * decreases and the effective density increases).
 *
 * By default, OLSR is used, but specifying a value of 2 for the protocol
 * will cause AODV to be used, and specifying a value of 3 will cause
 * DSDV to be used.
 *
 * By default, there are 10 source/sink data pairs sending UDP data
 * at an application rate of 2.048 Kb/s each.    This is typically done
 * at a rate of 4 64-byte packets per second.  Application data is
 * started at a random time between 50 and 51 seconds and continues
 * to the end of the simulation.
 *
 * The program outputs a few items:
 * - packet receptions are notified to stdout such as:
 *   <timestamp> <node-id> received one packet from <src-address>
 * - each second, the data reception statistics are tabulated and output
 *   to a comma-separated value (csv) file
 * - some tracing and flow monitor configuration that used to work is
 *   left commented inline in the program
 */

#include <fstream>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/flooding-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/applications-module.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;
using namespace dsr;

NS_LOG_COMPONENT_DEFINE ("manet-routing-compare");

class RoutingExperiment
{
public:
  RoutingExperiment ();
  void Run ();
  //static void SetMACParam (ns3::NetDeviceContainer & devices,
  //                                 int slotDistance);
  void CommandSetup (int argc, char **argv);

  std::string GetCSVFileName () const { return m_CSVfileName; }

private:
  void SetupPacketReceive (InetSocketAddress addr, Ptr<Node> node);
  void ReceivePacket (Ptr<Socket> socket);
  void CheckThroughput ();

  uint32_t port;
  uint32_t bytesTotal;
  uint32_t packetsReceived;

  std::string m_CSVfileName;
  int m_nSinks;
  int m_nNodes;
  std::string m_protocolName;
  double m_txp;
  bool m_traceMobility;
  uint32_t m_protocol;
  Time m_total_time;
  Time m_send_start;

private:
  std::vector<Ptr<Socket>> m_sinks;
};

RoutingExperiment::RoutingExperiment ()
  : port (9), // Discard port (RFC 863)
    bytesTotal (0),
    packetsReceived (0),
    m_CSVfileName ("manet-routing.output.csv"),
    m_nSinks (1),
    m_nNodes (100),
    m_protocolName(""),
    m_txp (7.5),
    m_traceMobility (false),
    m_protocol (2), // AODV
    m_total_time (Seconds (200.0)),
    m_send_start (Seconds (100.0))
{
}

static inline std::string
PrintReceivedPacket (Ptr<Socket> socket, Ptr<Packet> packet, Address senderAddress)
{
  std::ostringstream oss;

  oss << Simulator::Now ().GetSeconds () << " " << socket->GetNode ()->GetId ();

  if (InetSocketAddress::IsMatchingType (senderAddress))
    {
      InetSocketAddress addr = InetSocketAddress::ConvertFrom (senderAddress);
      oss << " received one packet from " << addr.GetIpv4 ();
    }
  else
    {
      oss << " received one packet!";
    }
  return oss.str ();
}

void
RoutingExperiment::ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address senderAddress;
  while ((packet = socket->RecvFrom (senderAddress)))
    {
      bytesTotal += packet->GetSize ();
      packetsReceived += 1;
      NS_LOG_UNCOND (PrintReceivedPacket (socket, packet, senderAddress));
    }
}

void
RoutingExperiment::CheckThroughput ()
{
  double kbs = (bytesTotal * 8.0) / 1000;
  bytesTotal = 0;

  std::ofstream out (m_CSVfileName.c_str (), std::ios::app);

  out << (Simulator::Now ()).GetSeconds () << ","
      << kbs << ","
      << packetsReceived << ","
      << m_nSinks << ","
      << m_nNodes << ","
      << m_protocolName << ","
      << m_txp << ""
      << std::endl;

  out.close ();
  packetsReceived = 0;
  Simulator::Schedule (Seconds (1.0), &RoutingExperiment::CheckThroughput, this);
}

void
RoutingExperiment::SetupPacketReceive (InetSocketAddress addr, Ptr<Node> node)
{
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket (node, tid);
  sink->Bind (addr);
  sink->SetRecvCallback (MakeCallback (&RoutingExperiment::ReceivePacket, this));

  NS_LOG_DEBUG("Bound receive to " << addr);

  m_sinks.push_back(sink);
}

void
RoutingExperiment::CommandSetup (int argc, char **argv)
{
  CommandLine cmd;
  cmd.AddValue ("CSVfileName", "The name of the CSV output file name", m_CSVfileName);
  cmd.AddValue ("traceMobility", "Enable mobility tracing", m_traceMobility);
  cmd.AddValue ("protocol", "1=OLSR;2=AODV;3=DSDV;4=Flooding;5=DSR", m_protocol);
  cmd.AddValue ("sinks", "The number of sinks", m_nSinks);
  cmd.AddValue ("nodes", "The number of nodes", m_nNodes);
  cmd.AddValue ("transmit-power", "The transmit power (default: 7.5)", m_txp);
  cmd.AddValue ("total-time", "The total simulation time (default: 200)", m_total_time);
  cmd.AddValue ("send-start", "The time at which packets will start sending (default: 100)", m_send_start);
  cmd.Parse (argc, argv);
}

int
main (int argc, char *argv[])
{
  RoutingExperiment experiment;
  experiment.CommandSetup (argc, argv);

  //blank out the last output file and write the column headers
  std::ofstream out (experiment.GetCSVFileName().c_str ());
  out << "SimulationSecond," <<
  "ReceiveRate," <<
  "PacketsReceived," <<
  "NumberOfSinks," <<
  "NumberOfNodes," <<
  "RoutingProtocol," <<
  "TransmissionPower" <<
  std::endl;
  out.close ();

  experiment.Run ();
}

void
RoutingExperiment::Run ()
{
  Packet::EnablePrinting ();

  std::string rate ("2048bps");
  uint32_t packet_size (64);
  std::string phyMode ("DsssRate11Mbps");
  std::string tr_name ("manet-routing-compare");
  int nodeSpeed = 20; //in m/s
  int nodePause = 0; //in s

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (packet_size));
  Config::SetDefault ("ns3::OnOffApplication::DataRate",  StringValue (rate));

  //Set Non-unicastMode rate to unicast mode
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

  NodeContainer adhocNodes;
  adhocNodes.Create (m_nNodes);

  // setting up wifi phy and channel using helpers
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a mac and disable rate control
  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue (phyMode),
                                "ControlMode", StringValue (phyMode));

  wifiPhy.Set ("TxPowerStart", DoubleValue (m_txp));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (m_txp));

  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer adhocDevices = wifi.Install (wifiPhy, wifiMac, adhocNodes);

  MobilityHelper mobilityAdhoc;
  int64_t streamIndex = 0; // used to get consistent mobility across scenarios

  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=300.0]"));
  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=300.0]"));

  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
  streamIndex += taPositionAlloc->AssignStreams (streamIndex);

  std::stringstream ssSpeed;
  ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed << "]";
  std::stringstream ssPause;
  ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";
  mobilityAdhoc.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                  "Speed", StringValue (ssSpeed.str ()),
                                  "Pause", StringValue (ssPause.str ()),
                                  "PositionAllocator", PointerValue (taPositionAlloc));
  mobilityAdhoc.SetPositionAllocator (taPositionAlloc);
  mobilityAdhoc.Install (adhocNodes);
  streamIndex += mobilityAdhoc.AssignStreams (adhocNodes, streamIndex);
  NS_UNUSED (streamIndex); // From this point, streamIndex is unused

  AodvHelper aodv;
  OlsrHelper olsr;
  DsdvHelper dsdv;
  DsrHelper dsr;
  FloodingHelper flood;
  DsrMainHelper dsrMain;
  Ipv4ListRoutingHelper list;
  InternetStackHelper internet;
  
  const int16_t priority = 100;

  switch (m_protocol)
    {
    case 1:
      list.Add (olsr, priority);
      m_protocolName = "OLSR";
      break;
    case 2:
      list.Add (aodv, priority);
      m_protocolName = "AODV";
      break;
    case 3:
      list.Add (dsdv, priority);
      m_protocolName = "DSDV";
      break;
    case 4:
      m_protocolName = "FLOODING";
      list.Add(flood, priority);
      break;
    case 5:
      m_protocolName = "DSR";
      break;
    default:
      NS_FATAL_ERROR ("No such protocol:" << m_protocol);
    }

  NS_LOG_INFO("Using " << m_protocolName << " to route messages");

  if (m_protocolName == "DSR")
    {
      internet.Install (adhocNodes);
      dsrMain.Install (dsr, adhocNodes);
    }
  else
    {
      internet.SetRoutingHelper (list);
      internet.Install (adhocNodes);
    }

  NS_LOG_INFO ("Assigning IP address");

  Ipv4AddressHelper addressAdhoc;
  addressAdhoc.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer adhocInterfaces = addressAdhoc.Assign (adhocDevices);

  Ptr<UniformRandomVariable> rnd = CreateObject<UniformRandomVariable> ();

  for (int i = 0; i < m_nSinks; i++)
    {
      const int target_id = i;
      const int source_id = i + m_nSinks;

      Ptr<Node> target_node = adhocNodes.Get (target_id);
      Ptr<Node> source_node = adhocNodes.Get (source_id);

      auto target = InetSocketAddress (adhocInterfaces.GetAddress (target_id), port);

      OnOffHelper onoff ("ns3::UdpSocketFactory", target);
      onoff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
      onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));

      ApplicationContainer temp = onoff.Install (source_node);
      temp.Start (Seconds (rnd->GetValue (m_send_start.GetSeconds(), m_send_start.GetSeconds() + 1.0)));
      temp.Stop (m_total_time);

      SetupPacketReceive (target, target_node);

      NS_LOG_INFO ("Node "
        << source_id
        << " (" << adhocInterfaces.GetAddress (source_id) << ")"
        << " will send packets to node " << target_id
        << " (" << adhocInterfaces.GetAddress (target_id) << ")");
    }

  std::stringstream ss;
  ss << m_nNodes;
  std::string nodes = ss.str ();

  std::stringstream ss2;
  ss2 << nodeSpeed;
  std::string sNodeSpeed = ss2.str ();

  std::stringstream ss3;
  ss3 << nodePause;
  std::string sNodePause = ss3.str ();

  std::stringstream ss4;
  ss4 << rate;
  std::string sRate = ss4.str ();

  //NS_LOG_INFO ("Configure Tracing.");
  //tr_name = tr_name + "_" + m_protocolName +"_" + nodes + "nodes_" + sNodeSpeed + "speed_" + sNodePause + "pause_" + sRate + "rate";

  //AsciiTraceHelper ascii;
  //Ptr<OutputStreamWrapper> osw = ascii.CreateFileStream ( (tr_name + ".tr").c_str());
  //wifiPhy.EnableAsciiAll (osw);
  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream (tr_name + ".mob"));

  //Ptr<FlowMonitor> flowmon;
  //FlowMonitorHelper flowmonHelper;
  //flowmon = flowmonHelper.InstallAll ();


  NS_LOG_INFO ("Run Simulation.");

  CheckThroughput ();

  Simulator::Stop (m_total_time);
  Simulator::Run ();

  //flowmon->SerializeToXmlFile ((tr_name + ".flowmon").c_str(), false, false);

  Simulator::Destroy ();
}
