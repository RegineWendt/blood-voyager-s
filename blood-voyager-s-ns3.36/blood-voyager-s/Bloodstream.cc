/* -*-  Mode: C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Universität zu Lübeck [GEYER]
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
 * Author: Regine Geyer <geyer@itm.uni-luebeck.de>
 */

#include "Bloodstream.h"

using namespace std;
namespace ns3 {

Bloodstream::Bloodstream ()
{
}

Bloodstream::~Bloodstream ()
{
  //this->m_offset_x;
  this->m_nanobots.erase (this->m_nanobots.begin (), this->m_nanobots.end ());
}

void
Bloodstream::initBloodstream (int vesselId, int streamId, int velocityfactor, double offsetX,
                              double offsetY, double angle)
{
  m_bloodvesselID = vesselId;
  m_currentStream = streamId; // id of stream
  m_velocity_factor = velocityfactor;
  m_offset_x = offsetX;
  m_offset_y = offsetY;
  m_offset_z = 0;
}

size_t
Bloodstream::CountNanbots (void)
{
  return this->m_nanobots.size ();
}

Ptr<Nanobot>
Bloodstream::GetNanobot (int index)
{
  list<Ptr<Nanobot>>::iterator i = this->m_nanobots.begin ();
  advance (i, index);
  return *i;
}

Ptr<Nanobot>
Bloodstream::RemoveNanobot (int index)
{
  return RemoveNanobot (GetNanobot (index));
}

Ptr<Nanobot>
Bloodstream::RemoveNanobot (Ptr<Nanobot> bot)
{
  m_nanobots.remove (bot);
  Vector v = bot->GetPosition ();
  v.x -= m_offset_x;
  v.y -= m_offset_y;
  v.z -= m_offset_z;
  bot->SetPosition (v);
  return bot;
}

void
Bloodstream::AddNanobot (Ptr<Nanobot> bot)
{
  m_nanobots.push_back (bot);
  bot->SetStream (m_currentStream);
  Vector v = bot->GetPosition ();
  v.x += m_offset_x;
  v.y += m_offset_y;
  v.z += m_offset_z;
  bot->SetPosition (v);
}

void
Bloodstream::SetAngle (double angle, double offsetX, double offsetY)
{
  if (angle != 0.0)
    {
      m_offset_z = offsetY;
      m_offset_x = offsetX * (cos (fmod ((angle - 90), 360) * M_PI / 180));
      m_offset_y = offsetX * (sin (fmod ((angle - 90), 360) * M_PI / 180));
    }
    else {
      m_offset_z = 0;
      m_offset_x = offsetY;
      m_offset_y = offsetX;
    }
}

void
Bloodstream::SortStream (void)
{
  m_nanobots.sort (ns3::Nanobot::Compare);
}

bool
Bloodstream::IsEmpty (void)
{
  return m_nanobots.size () <= 0;
}

double
Bloodstream::GetVelocity (void)
{
  return m_velocity;
}

void
Bloodstream::SetVelocity (double velocity)
{
  m_velocity = velocity * m_velocity_factor / 100.0;
}

} // namespace ns3