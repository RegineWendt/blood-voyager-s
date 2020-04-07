/* -*-  Mode: C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Universität zu Lübeck [WENDT]
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * Author: Regine Wendt <wendt@itm.uni-luebeck.de>
 */

#include "Nanobot.h"

namespace ns3 {
// TypeId Nanobot::GetTypeId (void)
// {
//   static TypeId tid = TypeId ("ns3::Nanobot")
//   .SetParent<Object> ()
//   .AddConstructor<Nanobot> ()
//   ;
//   return tid;
// }

Nanobot::Nanobot ()
{
  m_node = CreateObject<Node> ();
  //enables mobility
  MobilityHelper mobility;
  mobility.Install (m_node);

  m_length = 0.00001; //100nm
  m_width = 0.00001; //100nm
  m_stream_nb = 0;
  m_shouldChange = false;
  m_timeStep = Seconds (0.0);
}

Nanobot::~Nanobot ()
{
}

bool
Nanobot::Compare (Ptr<Nanobot> v1, Ptr<Nanobot> v2)
{
  if (v1->GetNanobotID () < v2->GetNanobotID ())
    {
      return true;
    }
  else
    {
      return false;
    }
}

int
Nanobot::GetNanobotID ()
{
  return m_nanobotID;
}

void
Nanobot::SetNanobotID (int value)
{
  m_nanobotID = value;
}

double
Nanobot::GetLength ()
{
  return m_length;
}

void
Nanobot::SetLength (double value)
{
  if (value < 0)
    {
      value = 0;
    }
  m_length = value;
}

double
Nanobot::GetWidth ()
{
  return m_width;
}

void
Nanobot::SetWidth (double value)
{
  if (value < 0)
    {
      value = 0;
    }
  m_width = value;
}

void
Nanobot::SetStream (int value)
{
  m_stream_nb = value;
}

int
Nanobot::GetStream ()
{
  return m_stream_nb;
}

bool
Nanobot::GetShouldChange ()
{
  return m_shouldChange;
}

void
Nanobot::SetShouldChange (bool value)
{
  m_shouldChange = value;
}

ns3::Time
Nanobot::GetTimeStep ()
{
  return m_timeStep;
}

void
Nanobot::SetTimeStep ()
{
  m_timeStep = Simulator::Now ();
}

Vector
Nanobot::GetPosition ()
{
  return m_node->GetObject<MobilityModel> ()->GetPosition ();
}

void
Nanobot::SetPosition (Vector value)
{
  m_node->GetObject<MobilityModel> ()->SetPosition (value);
}

} // namespace ns3
