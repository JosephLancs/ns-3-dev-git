/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "ns3/abort.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/test.h"
#include "adversary-mobility-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AdversaryMobilityModel");

NS_OBJECT_ENSURE_REGISTERED (AdversaryMobilityModel);
/*
 * 
 * 
 * rather than a list of waypoints, have a single target
 * 
 *
 */
TypeId
AdversaryMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::AdversaryMobilityModel")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<AdversaryMobilityModel> ()
  ;
  return tid;
}


AdversaryMobilityModel::AdversaryMobilityModel ()
  : m_has_waypoint (false)
  , m_has_set_position (false)
{
}

AdversaryMobilityModel::~AdversaryMobilityModel ()
{
}

void
AdversaryMobilityModel::DoDispose (void)
{
  MobilityModel::DoDispose ();
}

void
AdversaryMobilityModel::SetTarget (const Time& t, const Vector& v)
{
  const Time now = Simulator::Now ();

  const bool had_waypoint = m_has_waypoint;

  m_has_waypoint = true;
  m_current = Waypoint(t, v);

  m_velocity = m_current.position - m_position;
  m_velocity.x /= (m_current.time - now).GetSeconds();
  m_velocity.y /= (m_current.time - now).GetSeconds();
  m_velocity.z /= (m_current.time - now).GetSeconds();

  m_last_update = now;

  Update ();

  if (had_waypoint)
  {
    NotifyCourseChange ();
  }
}

void
AdversaryMobilityModel::Update (void) const
{
  NS_LOG_FUNCTION(this);

  NS_ASSERT(m_has_set_position);

  // Only move if we have a target
  if (!m_has_waypoint)
  {
    return;
  }

  const Time now = Simulator::Now ();
  const Time time_since_last_update = now - m_last_update;

  if (now >= m_current.time)
  {
    // If we should have reached the target by now, just set us there
    m_position = m_current.position;
  }
  else
  {
    Vector distance_to_travel = m_velocity;
    distance_to_travel.x *= time_since_last_update.GetSeconds();
    distance_to_travel.y *= time_since_last_update.GetSeconds();
    distance_to_travel.z *= time_since_last_update.GetSeconds();

    m_position = m_position + distance_to_travel;
  }

  m_last_update = now;

  // Reached target, stop moving
  if (CalculateDistance(m_position, m_current.position) == 0)
  {
    m_has_waypoint = false;
    m_velocity = Vector();
  }
}

Vector
AdversaryMobilityModel::DoGetPosition (void) const
{
  Update ();
  return m_position;
}

void
AdversaryMobilityModel::DoSetPosition (const Vector &position)
{
  const bool was_moving = m_velocity.GetLength() > 0;

  m_position = position;
  m_current = Waypoint();
  m_velocity = Vector();

  if (was_moving)
  {
    // This is only a course change if the node is actually moving
    NotifyCourseChange ();
  }

  m_has_set_position = true;
}

void
AdversaryMobilityModel::EndMobility (void)
{
  m_has_waypoint = false;
  m_velocity = Vector();
}

Vector
AdversaryMobilityModel::DoGetVelocity (void) const
{
  return m_velocity;
}

} // namespace ns3
