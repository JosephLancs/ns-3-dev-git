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
#ifndef ADVERSARY_MOBILITY_MODEL_H
#define ADVERSARY_MOBILITY_MODEL_H

#include <stdint.h>
#include <deque>
#include "mobility-model.h"
#include "ns3/vector.h"
#include "waypoint.h"


namespace ns3 {

/**
 * \ingroup mobility
 * \brief Random waypoint mobility model.
 *
 * Each object starts by pausing at time zero for the duration governed
 * by the random variable "Pause".  After pausing, the object will pick 
 * a new waypoint (via the PositionAllocator) and a new random speed 
 * via the random variable "Speed", and will begin moving towards the 
 * waypoint at a constant speed.  When it reaches the destination, 
 * the process starts over (by pausing).
 *
 * This mobility model enforces no bounding box by itself; the 
 * PositionAllocator assigned to this object will bound the movement.
 * If the user fails to provide a pointer to a PositionAllocator to
 * be used to pick waypoints, the simulation program will assert.
 *
 * The implementation of this model is not 2d-specific. i.e. if you provide
 * a 3d random waypoint position model to this mobility model, the model 
 * will still work. There is no 3d position allocator for now but it should
 * be trivial to add one.
 */
class AdversaryMobilityModel : public MobilityModel
{
public:
  /**
   * Register this type with the TypeId system.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Create a path with no waypoints at location (0,0,0).
   */
  AdversaryMobilityModel ();
  virtual ~AdversaryMobilityModel ();

  /**
   * \param waypoint waypoint to append to the object path.
   *
   * Add a waypoint to the path of the object. The time must
   * be greater than the previous waypoint added, otherwise
   * a fatal error occurs. The first waypoint is set as the
   * current position with a velocity of zero.
   *
   */
  void AddWaypoint (const Waypoint &waypoint);

  /**
   * @brief Set the Target object
   * 
   * @param v vector of target
   * @return true 
   * @return false 
   */
  bool SetTarget (Vector v);

  /**
   * Get the waypoint that this object is traveling towards.
   */
  Waypoint GetNextWaypoint (void) const;

  /**
   * Get the number of waypoints left for this object, excluding
   * the next one.
   */
  uint32_t WaypointsLeft (void) const;

  /**
   * Clear any existing waypoints and set the current waypoint
   * time to infinity. Calling this is only an optimization and
   * not required. After calling this function, adding waypoints
   * behaves as it would for a new object.
   */
  void EndMobility (void);

private:
  /**
   * Update the underlying state corresponding to the stored waypoints
   */
  virtual void Update (void) const;
  /**
   * \brief The dispose method.
   * 
   * Subclasses must override this method.
   */
  virtual void DoDispose (void);
  /**
   * \brief Get current position.
   * \return A vector with the current position of the node.  
   */
  virtual Vector DoGetPosition (void) const;
  /**
   * \brief Sets a new position for the node  
   * \param position A vector to be added as the new position
   */
  virtual void DoSetPosition (const Vector &position);
  /**
   * \brief Returns the current velocity of a node
   * \return The velocity vector of a node. 
   */
  virtual Vector DoGetVelocity (void) const;

protected:
  /**
   * \brief This variable is set to true if there are no waypoints in the std::deque
   */
  bool m_first;
  /**
   * \brief If true, course change updates are only notified when position
   * is calculated.
   */
  bool m_lazyNotify;
  /**
   * \brief If true, calling SetPosition with no waypoints creates a waypoint
   */
  bool m_initialPositionIsWaypoint;
  /**
   * \brief The double ended queue containing the ns3::Waypoint objects
   */
  mutable std::deque<Waypoint> m_waypoints;
  /**
   * \brief The ns3::Waypoint currently being used
   */
  mutable Waypoint m_current;
  /**
   * \brief The next ns3::Waypoint in the deque
   */
  mutable Waypoint m_next;
  /**
   * \brief The current velocity vector
   */
  mutable Vector m_velocity;
};

} // namespace ns3

#endif /* RANDOM_WAYPOINT_MOBILITY_MODEL_H */
