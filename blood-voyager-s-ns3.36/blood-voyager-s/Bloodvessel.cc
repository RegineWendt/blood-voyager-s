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
 * Author: Regine Geyer <geyer@itm.uni-luebeck.de>, Chris Deter <chris.deter@student.uni-luebeck.de>
 */

#include "Bloodvessel.h"
#include <random>
#include <functional>

using namespace std;
namespace ns3 {

auto randomIntegerBetweenZeroAndOne =
    std::bind (std::uniform_int_distribution<> (0, 1), std::default_random_engine ());

TypeId
Bloodvessel::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::Bloodvessel").SetParent<Object> ().AddConstructor<Bloodvessel> ();
  return tid;
}

Bloodvessel::Bloodvessel ()
{
  m_deltaT = 1;
  initStreams ();
  m_changeStreamSet = true;
  m_basevelocity = 0;
  m_currentStream = 0;
}

Bloodvessel::~Bloodvessel ()
{
  /*  for (int i = 0; i < m_numberOfStreams; i++)
    {
      m_nanobots[i].erase (m_nanobots[i].begin (), m_nanobots[i].end ());
    }
    */
}

void
Bloodvessel::SetPrinter (Ptr<PrintNanobots> printer)
{
  this->printer = printer;
}

void
Bloodvessel::Start ()
{
  m_start = true;
  Simulator::Schedule (Seconds (0.0), &Step, Ptr<Bloodvessel> (this));
}

void
Bloodvessel::Step (Ptr<Bloodvessel> bloodvessel)
{
  if (bloodvessel->GetbloodvesselID () == 1)
    {
      static clock_t now, old = clock ();
      old = now;
      now = clock ();
      cout << (now - old) / 1000000 << " -> " << Simulator::Now () << endl;
    }
  bloodvessel->TranslateNanobots ();
}

Ptr<UniformRandomVariable>
Bloodvessel::getRandomObjectBetween (double min, double max)
{
  Ptr<UniformRandomVariable> random = CreateObject<UniformRandomVariable> ();
  random->SetAttribute ("Min", DoubleValue (min));
  random->SetAttribute ("Max", DoubleValue (max));
  return random;
}

bool
Bloodvessel::getRandomBoolean ()
{
  return randomIntegerBetweenZeroAndOne ();
}

Vector
Bloodvessel::SetPosition (Vector nbv, double distance, double angle, int bloodvesselType,
                          double startPosZ)
{
  //Check vessel direction and move according to distance.
  //right
  if (angle == 0.00 && bloodvesselType != ORGAN)
    {
      nbv.x += distance;
    }
  //left
  else if (angle == -180.00 || angle == 180.00)
    {
      nbv.x -= distance;
    }
  //down
  else if (angle == -90.00)
    {
      nbv.y -= distance;
    }
  //up
  else if (angle == 90.00)
    {
      nbv.y += distance;
    }
  //back
  else if (angle == 0.00 && bloodvesselType == ORGAN && startPosZ == 2)
    {
      nbv.z -= distance;
    }
  //front
  else if (angle == 0.00 && bloodvesselType == ORGAN && startPosZ == -2)
    {
      nbv.z += distance;
    }
  //right up
  else if ((0.00 < angle && angle < 90.00) || (-90.00 < angle && angle < 0.00) ||
           (90.00 < angle && angle < 180.00) || (-180.00 < angle && angle < -90.00))
    {
      nbv.x += distance * (cos (fmod ((angle), 360) * M_PI / 180));
      nbv.y += distance * (sin (fmod ((angle), 360) * M_PI / 180));
    }
  return nbv;
}

void
Bloodvessel::TranslateNanobots ()
{
  if (m_start)
    {
      Simulator::Schedule (Seconds (m_deltaT), &Bloodvessel::Step, Ptr<Bloodvessel> (this));
      m_start = false;
      return;
    }
  if (this->IsEmpty ())
    {
      Simulator::Schedule (Seconds (m_deltaT), &Bloodvessel::Step, Ptr<Bloodvessel> (this));
      return;
    }
  static int loop = 1;
  if (loop == 2)
    {
      loop = 0;
    }
  //Change streams only in organs
  if (loop == 0 && m_changeStreamSet == true && this->GetBloodvesselType () == ORGAN)
    {
      ChangeStream ();
    }
  //Translate their position every timestep
  TranslatePosition (m_deltaT);
  loop++;
  Simulator::Schedule (Seconds (m_deltaT), &Bloodvessel::Step, Ptr<Bloodvessel> (this));
}

bool
Bloodvessel::moveNanobot (Ptr<Nanobot> nb, int i, int randVelocityOffset, bool direction, double dt)
{
  double distance = 0.0;
  double velocity = m_bloodstreams[i]->GetVelocity ();
  if (direction)
    {
      distance = (velocity - ((velocity / 100) * randVelocityOffset)) * dt;
    }
  else
    {
      distance = (velocity + ((velocity / 100) * randVelocityOffset)) * dt;
    }
  //Check vessel direction and move accordingly.
  Vector nbv = SetPosition (nb->GetPosition (), distance, GetBloodvesselAngle (),
                            GetBloodvesselType (), m_startPositionBloodvessel.z);
  nb->SetPosition (nbv);
  nb->SetTimeStep ();
  double nbx = nb->GetPosition ().x - m_startPositionBloodvessel.x;
  double nby = nb->GetPosition ().y - m_startPositionBloodvessel.y;
  double length = sqrt (nbx * nbx + nby * nby);
  //check if position exceeds bloodvessel
  return (length > m_bloodvesselLength || (nbv.z < -2 && m_angle == 0) ||
          (nbv.z > 2 && m_angle == 0));
}

void
Bloodvessel::TranslatePosition (double dt)
{
  list<Ptr<Nanobot>> print;
  list<Ptr<Nanobot>> reachedEnd;

  auto randomVelocityOffset =
      std::bind (std::uniform_int_distribution<> (0, 11), std::default_random_engine ());
  // for every stream of the vessel
  for (int i = 0; i < m_numberOfStreams; i++)
    {
      // for every nanobot of the stream
      for (uint j = 0; j < m_bloodstreams[i]->CountNanbots (); j++)
        {
          Ptr<Nanobot> nb = m_bloodstreams[i]->GetNanobot (j);
          //move only nanobots that have not already been translated by another vessel
          if (nb->GetTimeStep () < Simulator::Now ())
            {
              // has nanobot reached end after moving
              if (moveNanobot (nb, i, randomVelocityOffset (), getRandomBoolean (), dt))
                {
                  reachedEnd.push_back (nb);
                }
              else
                {
                  print.push_back (nb);
                  //PrintNanobot (nb, this);
                }
            } //if timestep not from old
        } //inner for
      //Push Nanobots that reached the end of the bloodvessel to next vessel.
      if (reachedEnd.size () > 0)
        {
          TransposeNanobots (reachedEnd, i);
        }
      reachedEnd.clear ();
    } //outer for
  printer->PrintSomeNanobots (print, this->GetbloodvesselID ());
} //Function

void
Bloodvessel::ChangeStream ()
{
  if (m_numberOfStreams > 1)
    {
      Ptr<UniformRandomVariable> nb_randomize = getRandomObjectBetween (0, m_numberOfStreams);
      //set half of the nanobots randomly to change
      for (int i = 0; i < m_numberOfStreams; i++)
        {
          for (uint j = 0; j < m_bloodstreams[i]->CountNanbots (); j++) //in every stream
            {
              if (getRandomBoolean ())
                { // 50 / 50 Chance for every nanobot to change
                  m_bloodstreams[i]->GetNanobot (j)->SetShouldChange (true);
                }
            }
        }
      // after all nanobots that should change are flagged, do change
      for (int i = 0; i < m_numberOfStreams; i++)
        {
          // Random Number that is "1" or "-1"
          int direction = getRandomBoolean () == true ? -1 : 1;
          if (i == 0)
            { //Special Case 1: outer lane left -> go to middle
              direction = 1;
            }
          else if (i + 1 >= m_numberOfStreams)
            { //Special Case 2: outer lane right -> go to middle
              direction = -1;
            }
          // Move randomly left or right
          DoChangeStreamIfPossible (i, i + direction);
        }
    }
}

void
Bloodvessel::DoChangeStreamIfPossible (int curStream, int desStream)
{
  list<Ptr<Nanobot>> canChange;
  canChange.clear ();
  for (uint j = 0; j < m_bloodstreams[curStream]->CountNanbots (); j++)
    {
      if (m_bloodstreams[curStream]->GetNanobot (j)->GetShouldChange ())
        {
          //set should change back to false
          m_bloodstreams[curStream]->GetNanobot (j)->SetShouldChange (false);
          m_bloodstreams[desStream]->AddNanobot (m_bloodstreams[curStream]->RemoveNanobot (j));
        }
    }
  //Sort all Nanobots by ID
  m_bloodstreams[desStream]->SortStream ();
}

bool
Bloodvessel::transposeNanobot (Ptr<Nanobot> botToTranspose, Ptr<Bloodvessel> thisBloodvessel,
                               Ptr<Bloodvessel> nextBloodvessel, int stream)
{
  Vector stopPositionOfVessel = thisBloodvessel->GetStopPositionBloodvessel ();
  Vector nanobotPosition = botToTranspose->GetPosition ();
  double distance = sqrt (pow (nanobotPosition.x - stopPositionOfVessel.x, 2) +
                          pow (nanobotPosition.y - stopPositionOfVessel.y, 2) +
                          pow (nanobotPosition.z - stopPositionOfVessel.z, 2));
  distance = distance / thisBloodvessel->m_bloodstreams[stream]->GetVelocity () *
             nextBloodvessel->m_bloodstreams[stream]->GetVelocity ();
  botToTranspose->SetPosition (nextBloodvessel->GetStartPositionBloodvessel ());
  Vector rmp = SetPosition (
      botToTranspose->GetPosition (), distance, nextBloodvessel->GetBloodvesselAngle (),
      nextBloodvessel->GetBloodvesselType (), thisBloodvessel->GetStopPositionBloodvessel ().z);
  botToTranspose->SetPosition (rmp);
  double nbx = botToTranspose->GetPosition ().x - nextBloodvessel->GetStartPositionBloodvessel ().x;
  double nby = botToTranspose->GetPosition ().y - nextBloodvessel->GetStartPositionBloodvessel ().y;
  double length = sqrt (nbx * nbx + nby * nby);
  //check if position exceeds bloodvessel
  return length > nextBloodvessel->GetbloodvesselLength () || rmp.z < -2 || rmp.z > 2;
}

void
Bloodvessel::TransposeNanobots (list<Ptr<Nanobot>> reachedEnd, int stream)
{
  list<Ptr<Nanobot>> print1;
  list<Ptr<Nanobot>> print2;
  list<Ptr<Nanobot>> reachedEndAgain;
  Ptr<UniformRandomVariable> rv = getRandomObjectBetween (0, 4);
  int onetwo;
  for (const Ptr<Nanobot> &botToTranspose : reachedEnd)
    {
      onetwo = floor (rv->GetValue ());
      // Remove Nanobot from actual bloodstream
      m_bloodstreams[stream]->RemoveNanobot (botToTranspose);
      //Bias for Vessels that deliver blood to organs - 75% go directly forward in the main vessel - hard coded
      if (m_nextBloodvessel2 != 0 && (m_nextBloodvessel2->GetbloodvesselID () == 6 ||
                                      m_nextBloodvessel2->GetbloodvesselID () == 32 ||
                                      m_nextBloodvessel2->GetbloodvesselID () == 35))
        {
          if (onetwo == 1)
            {
              onetwo = 2;
            }
        }
      // TODO CD:  Umbauen auf m_nextBloodVessel->getNextBloodvessel->isOrgan()?
      if (m_nextBloodvessel1->GetbloodvesselID () == 3 ||
          m_nextBloodvessel1->GetbloodvesselID () == 7 ||
          m_nextBloodvessel1->GetbloodvesselID () == 12 ||
          m_nextBloodvessel1->GetbloodvesselID () == 24 ||
          m_nextBloodvessel1->GetbloodvesselID () == 25 ||
          m_nextBloodvessel1->GetbloodvesselID () == 37 ||
          m_nextBloodvessel1->GetbloodvesselID () == 43 ||
          m_nextBloodvessel1->GetbloodvesselID () == 45)
        {
          if (onetwo == 2)
            {
              onetwo = 1;
            }
        }
      if (m_nextBloodvessel2 != 0 && (onetwo == 0 || onetwo == 1))
        {
          if (transposeNanobot (botToTranspose, this, m_nextBloodvessel2, stream))
            {
              reachedEndAgain.push_back (botToTranspose);
              m_nextBloodvessel2->TransposeNanobots (reachedEndAgain, stream);
              reachedEndAgain.clear ();
            }
          else
            {
              m_nextBloodvessel2->m_bloodstreams[stream]->AddNanobot (botToTranspose);
              print2.push_back (botToTranspose);
            }
        }
      else if ((onetwo == 2 || onetwo == 3) || m_nextBloodvessel2 == 0)
        {
          if (transposeNanobot (botToTranspose, this, m_nextBloodvessel1, stream))
            {
              reachedEndAgain.push_back (botToTranspose);
              m_nextBloodvessel1->TransposeNanobots (reachedEndAgain, stream);
              reachedEndAgain.clear ();
            }
          else
            {
              m_nextBloodvessel1->m_bloodstreams[stream]->AddNanobot (botToTranspose);
              print1.push_back (botToTranspose);
            }
        }
    } // for
  printer->PrintSomeNanobots (print1, m_nextBloodvessel1->GetbloodvesselID ());
  if (m_nextBloodvessel2)
    {
      printer->PrintSomeNanobots (print2, m_nextBloodvessel2->GetbloodvesselID ());
    }
}

//HELPER
void
Bloodvessel::PrintNanobotsOfVessel ()
{
  for (uint j = 0; j < m_bloodstreams.size (); j++)
    {
      for (uint i = 0; i < m_bloodstreams[j]->CountNanbots (); i++)
        {
          printer->PrintNanobot (m_bloodstreams[j]->GetNanobot (i), GetbloodvesselID ());
        }
    }
}

void
Bloodvessel::initStreams ()
{
  int i;
  Ptr<Bloodstream> stream;
  for (i = 0; i < stream_definition_size; i++)
    {
      stream = CreateObject<Bloodstream> ();
      stream->initBloodstream (m_bloodvesselID, i, stream_definition[i][0],
                               stream_definition[i][1] / 10.0, stream_definition[i][2] / 10.0,
                               GetBloodvesselAngle ());
      m_bloodstreams.push_back (stream);
    }
  m_numberOfStreams = stream_definition_size;
}

double
Bloodvessel::CalcLength ()
{
  if (GetBloodvesselType () == ORGAN)
    {
      return 4;
    }
  else
    {
      Vector m_start = GetStartPositionBloodvessel ();
      Vector m_end = GetStopPositionBloodvessel ();
      double l = sqrt (pow (m_end.x - m_start.x, 2) + pow (m_end.y - m_start.y, 2));
      return l;
    }
}

double
Bloodvessel::CalcAngle ()
{
  Vector m_start = GetStartPositionBloodvessel ();
  Vector m_end = GetStopPositionBloodvessel ();
  double x = m_end.x - m_start.x;
  double y = m_end.y - m_start.y;
  return atan2 (y, x) * 180 / M_PI;
}

void
Bloodvessel::InitBloodstreamLengthAngleAndVelocity (double velocity)
{
  int i;
  double length = CalcLength ();
  m_bloodvesselLength = length < 0 ? 10000 : length;
  m_angle = CalcAngle ();
  if (velocity >= 0)
    {
      m_basevelocity = velocity;
      int maxLength = 0;
      // calulate the width per stream
      for (i = 0; i < m_numberOfStreams; i++)
        {
          if (maxLength < stream_definition[i][1])
            {
              maxLength = stream_definition[i][1];
            }
          if (maxLength < stream_definition[i][2])
            {
              maxLength = stream_definition[i][2];
            }
        }
      double offset = m_vesselWidth / 2.0 / maxLength;
      // Set velocity, angle and position offset
      for (i = 0; i < m_numberOfStreams; i++)
        {
          m_bloodstreams[i]->SetVelocity (m_basevelocity);
          m_bloodstreams[i]->SetAngle (m_angle, stream_definition[i][1] * offset,
                                       stream_definition[i][2] * offset);
        }
    }
}

//GETTER AND SETTER

bool
Bloodvessel::IsEmpty ()
{
  bool empty = true;
  for (int i = 0; i < m_numberOfStreams; i++)
    {
      empty = empty && m_bloodstreams[i]->IsEmpty ();
    }
  return empty;
}

int
Bloodvessel::GetbloodvesselID ()
{
  return m_bloodvesselID;
}

void
Bloodvessel::SetBloodvesselID (int b_id)
{
  m_bloodvesselID = b_id;
}

double
Bloodvessel::GetBloodvesselAngle ()
{
  return m_angle;
}

int
Bloodvessel::GetNumberOfStreams ()
{
  return m_numberOfStreams;
}

Ptr<Bloodstream>
Bloodvessel::GetStream (int id)
{
  return m_bloodstreams[id];
}

double
Bloodvessel::GetbloodvesselLength ()
{
  return m_bloodvesselLength;
}

void
Bloodvessel::SetVesselWidth (double value)
{
  m_vesselWidth = value;
}

void
Bloodvessel::AddNanobotToStream (unsigned int streamID, Ptr<Nanobot> bot)
{
  m_bloodstreams[streamID]->AddNanobot (bot);
}

BloodvesselType
Bloodvessel::GetBloodvesselType ()
{
  return m_bloodvesselType;
}

void
Bloodvessel::SetBloodvesselType (BloodvesselType value)
{
  m_bloodvesselType = value;
}

void
Bloodvessel::SetNextBloodvessel1 (Ptr<Bloodvessel> value)
{
  m_nextBloodvessel1 = value;
}

void
Bloodvessel::SetNextBloodvessel2 (Ptr<Bloodvessel> value)
{
  m_nextBloodvessel2 = value;
}

Vector
Bloodvessel::GetStartPositionBloodvessel ()
{
  return m_startPositionBloodvessel;
}

void
Bloodvessel::SetStartPositionBloodvessel (Vector value)
{
  m_startPositionBloodvessel = value;
}

Vector
Bloodvessel::GetStopPositionBloodvessel ()
{
  return m_stopPositionBloodvessel;
}

void
Bloodvessel::SetStopPositionBloodvessel (Vector value)
{
  m_stopPositionBloodvessel = value;
}
} // namespace ns3