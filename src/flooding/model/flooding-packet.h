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
 *      NS-2 Flooding model developed by the CMU/MONARCH group and optimized and
 *      tuned by Samir Das and Mahesh Marina, University of Cincinnati;
 *
 *      Flooding-UU implementation by Erik Nordström of Uppsala University
 *      http://core.it.uu.se/core/index.php/Flooding-UU
 *
 * Authors: Elena Buchatskaia <borovkovaes@iitp.ru>
 *          Pavel Boyko <boyko@iitp.ru>
 */
#ifndef FLOODINGPACKET_H
#define FLOODINGPACKET_H

#include <iostream>
#include "ns3/header.h"
#include "ns3/enum.h"
#include "ns3/ipv4-address.h"
#include <map>
#include "ns3/nstime.h"

namespace ns3 {
namespace flooding {

/**
* \ingroup FLOODING
* \brief MessageType enumeration
*/
enum MessageType
{
  FLOODINGTYPE_FLOODING  = 1
};

/**
* \ingroup FLOODING
* \brief FLOODING types
*/
class TypeHeader : public Header
{
public:
  /**
   * constructor
   * \param t the FLOODING RREQ type
   */
  TypeHeader (MessageType t = FLOODINGTYPE_FLOODING);

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  /**
   * \returns the type
   */
  MessageType Get () const
  {
    return m_type;
  }
  /**
   * Check that type if valid
   * \returns true if the type is valid
   */
  bool IsValid () const
  {
    return m_valid;
  }
  /**
   * \brief Comparison operator
   * \param o header to compare
   * \return true if the headers are equal
   */
  bool operator== (TypeHeader const & o) const;
private:
  MessageType m_type; ///< type of the message
  bool m_valid; ///< Indicates if the message is valid
};

/**
  * \brief Stream output operator
  * \param os output stream
  * \return updated stream
  */
std::ostream & operator<< (std::ostream & os, TypeHeader const & h);

/** - TODO: change header to flooding header
* \ingroup FLOODING
* \brief   Route Request (RREQ) Message Format
  \verbatim
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |     Type      |J|R|G|D|U|   Reserved          |   Hop Count   |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                            RREQ ID                            |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                    Destination IP Address                     |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                  Destination Sequence Number                  |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                    Originator IP Address                      |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                  Originator Sequence Number                   |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  \endverbatim
*/
class RreqHeader : public Header
{
public:
  /**
   * constructor
   *
   * \param flags the message flags (0)
   * \param reserved the reserved bits (0)
   * \param hopCount the hop count
   * \param requestID the request ID
   * \param dst the destination IP address
   * \param dstSeqNo the destination sequence number
   * \param origin the origin IP address
   * \param originSeqNo the origin sequence number
   */
   RreqHeader (uint8_t flags = 0, uint8_t reserved = 0, uint8_t hopCount = 0,
              uint32_t requestID = 0, Ipv4Address dst = Ipv4Address (),
              uint32_t dstSeqNo = 0, Ipv4Address origin = Ipv4Address (),
              uint32_t originSeqNo = 0);

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  // Fields
  /**
   * \brief Set the hop count
   * \param count the hop count
   */
  void SetHopCount (uint8_t count)
  {
    m_hopCount = count;
  }
  /**
   * \brief Get the hop count
   * \return the hop count
   */
  uint8_t GetHopCount () const
  {
    return m_hopCount;
  }
  /**
   * \brief Set the request ID
   * \param id the request ID
   */
  void SetId (uint32_t id)
  {
    m_requestID = id;
  }
  /**
   * \brief Get the request ID
   * \return the request ID
   */
  uint32_t GetId () const
  {
    return m_requestID;
  }
  /**
   * \brief Set the destination address
   * \param a the destination address
   */
  void SetDst (Ipv4Address a)
  {
    m_dst = a;
  }
  /**
   * \brief Get the destination address
   * \return the destination address
   */
  Ipv4Address GetDst () const
  {
    return m_dst;
  }
  /**
   * \brief Set the destination sequence number
   * \param s the destination sequence number
   */
  void SetDstSeqno (uint32_t s)
  {
    m_dstSeqNo = s;
  }
  /**
   * \brief Get the destination sequence number
   * \return the destination sequence number
   */
  uint32_t GetDstSeqno () const
  {
    return m_dstSeqNo;
  }
  /**
   * \brief Set the origin address
   * \param a the origin address
   */
  void SetOrigin (Ipv4Address a)
  {
    m_origin = a;
  }
  /**
   * \brief Get the origin address
   * \return the origin address
   */
  Ipv4Address GetOrigin () const
  {
    return m_origin;
  }
  /**
   * \brief Set the origin sequence number
   * \param s the origin sequence number
   */
  void SetOriginSeqno (uint32_t s)
  {
    m_originSeqNo = s;
  }
  /**
   * \brief Get the origin sequence number
   * \return the origin sequence number
   */
  uint32_t GetOriginSeqno () const
  {
    return m_originSeqNo;
  }

  // Flags
  /**
   * \brief Set the gratuitous RREP flag
   * \param f the gratuitous RREP flag
   */
  void SetGratuitousRrep (bool f);
  /**
   * \brief Get the gratuitous RREP flag
   * \return the gratuitous RREP flag
   */
  bool GetGratuitousRrep () const;
  /**
   * \brief Set the Destination only flag
   * \param f the Destination only flag
   */
  void SetDestinationOnly (bool f);
  /**
   * \brief Get the Destination only flag
   * \return the Destination only flag
   */
  bool GetDestinationOnly () const;
  /**
   * \brief Set the unknown sequence number flag
   * \param f the unknown sequence number flag
   */
  void SetUnknownSeqno (bool f);
  /**
   * \brief Get the unknown sequence number flag
   * \return the unknown sequence number flag
   */
  bool GetUnknownSeqno () const;

  /**
   * \brief Comparison operator
   * \param o RREQ header to compare
   * \return true if the RREQ headers are equal
   */
  bool operator== (RreqHeader const & o) const;
private:
  uint8_t        m_flags;          ///< |J|R|G|D|U| bit flags, see RFC
  uint8_t        m_reserved;       ///< Not used (must be 0)
  uint8_t        m_hopCount;       ///< Hop Count
  uint32_t       m_requestID;      ///< RREQ ID
  Ipv4Address    m_dst;            ///< Destination IP Address
  uint32_t       m_dstSeqNo;       ///< Destination Sequence Number
  Ipv4Address    m_origin;         ///< Originator IP Address
  uint32_t       m_originSeqNo;    ///< Source Sequence Number
};

/**
  * \brief Stream output operator
  * \param os output stream
  * \return updated stream
  */
std::ostream & operator<< (std::ostream & os, RreqHeader const &);

}  // namespace FLOODING
}  // namespace ns3

#endif /* FLOODINGPACKET_H */
