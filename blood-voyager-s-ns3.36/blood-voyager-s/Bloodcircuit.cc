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

#include "Bloodcircuit.h"
#include <fstream>

/**
 * This function sets the Bloodvessel map of the human body of a female: hight 1.72m, weight 69kg, age 29.
 */

namespace ns3 {

Bloodcircuit::Bloodcircuit (unsigned int numberOfNanobots, unsigned int injectionVessel, Ptr<PrintNanobots> printer)
{
  // initialise map with bloodvesselinformation
  m_bloodvessels = map<int, Ptr<Bloodvessel>> ();
  this->printer = printer;
  // loading vasculature via csv
  std::ifstream infile{"vasculature.csv"};
  if (infile.good ())
    {
      std::vector<int> numbers;
      numbers.resize (valuesPerLine);
      std::string buffer;
      buffer.reserve (64);
      int errorflag = 0;
      while (infile.good ())
        {
          for (auto &&elem : numbers)
            {
              if (std::getline (infile, buffer, ','))
                {
                  elem = std::stoi (buffer);
                }
              else
                {
                  elem = 0;
                  errorflag = 1;
                }
            }
          if (errorflag == 0)
            {
              AddVesselData (numbers[0], (BloodvesselType) numbers[1],
                             Vector (numbers[2], numbers[3], numbers[4]),
                             Vector (numbers[5], numbers[6], numbers[7]));
            }
        }
      cout << "load CSV - done" << endl;
    }
  else
    { // Old way of loading data
      cout << "NO VALID CSV FILE FOUND! " << endl;
      cout << "please provide a valid 'vasculatures.csv'" << endl;
    }

  //Create Bloodcircuit with all Bloodvessels.
  //
  ConnectBloodvessels ();
  //Inject Nanobots here!
  //Places x Bots, randomly on Streams at specific vessel injected.
  //If you choose other values, the nanobots all get injected at the same coordinates
  if (m_bloodvessels.size() > 1){
    InjectNanobots (numberOfNanobots, m_bloodvessels[injectionVessel <  m_bloodvessels.size() ?  injectionVessel : m_bloodvessels.size() -1]);
  }
}

//Starts the simulation in a bloodvessel
void
Starter (Ptr<Bloodvessel> vessel)
{
  vessel->Start ();
}

int
Bloodcircuit::BeginnSimulation (unsigned int simulationDuration, unsigned int numOfNanobots, unsigned int injectionVessel)
{

  //execution time
  clock_t start, finish;
  start = clock ();
  Ptr<PrintNanobots> printNano = new PrintNanobots ();
  //Create the bloodcircuit
  Bloodcircuit circuit (numOfNanobots, injectionVessel, printNano);
  //Get the map of the bloodcircuit
  map<int, ns3::Ptr<ns3::Bloodvessel>> circuitMap = circuit.GetBloodcircuit ();
  if(circuitMap.size() > 1){
    // Schedule and run the Simulation in each bloodvessel
    for (unsigned int i = 1; i < circuitMap.size() + 1; i++)
      {
                Simulator::Schedule (Seconds (0.0), &Starter, circuitMap[i]); 
      }
    //Stop simulation after specific time
    Simulator::Stop (Seconds (simulationDuration + 1));
    Simulator::Run ();
    Simulator::Destroy ();
    //output execution time
    finish = clock ();
    cout << "Dauer: " << simulationDuration << "s " << numOfNanobots << "NB -> "
        << (finish - start) / 1000000 << "s ------------------------" << endl;
    cout << "Injected Vessel: " << injectionVessel << endl;
    return 0;
  } else {
     cout << "Not enough vessels for simulation! Please check 'vasculature.csv'" << endl;
     return 1;
  }
}

Bloodcircuit::~Bloodcircuit ()
{
  m_bloodvessels.clear ();
}

map<int, ns3::Ptr<ns3::Bloodvessel>>
Bloodcircuit::GetBloodcircuit ()
{
  return m_bloodvessels;
}

Vector
Bloodcircuit::CalcDirectionVectorNorm (Ptr<Bloodvessel> m_bloodvessel)
{
  Vector start = m_bloodvessel->GetStartPositionBloodvessel ();
  Vector end = m_bloodvessel->GetStopPositionBloodvessel ();
  double x = end.x - start.x;
  double y = end.y - start.y;
  double z = end.z - start.z;
  double vectorLength = sqrt (pow (x, 2) + pow (y, 2) + pow (z, 2));
  x = x / vectorLength;
  y = y / vectorLength;
  z = z / vectorLength;
  return Vector (x, y, z);
}

void
Bloodcircuit::InjectNanobots (int numberOfNanobots, Ptr<Bloodvessel> bloodvessel)
{
  int nanobotGroupSize = 10;
  Ptr<UniformRandomVariable> distribute_randomly =
      bloodvessel->getRandomObjectBetween (0, bloodvessel->GetNumberOfStreams ());
  Vector m_coordinateStart = bloodvessel->GetStartPositionBloodvessel ();
  int intervall =
      (numberOfNanobots >= nanobotGroupSize) //IF if equal or more than 10 Nanobots are injected,
          ? div (numberOfNanobots, nanobotGroupSize)
                .quot // THEN Divide number of Nanobots into 10 equaly sized groups + remainder
          : numberOfNanobots; // ELSE put them on beginning of the bloodvessel in one point.
  //Calculate the normalized direction vector of the start vessel.
  Vector m_direction = CalcDirectionVectorNorm (bloodvessel);
  //Set direction intervall as 1/10 of the normalized direction vector.
  Vector directionIntervall =
      Vector (m_direction.x / nanobotGroupSize, m_direction.y / nanobotGroupSize,
              m_direction.z / nanobotGroupSize);
  unsigned int group = 0; // Group 0 to 9
  //Distribute Nanobots in 10 groups over the beginning of the start vessel.
  for (int i = 1; i <= numberOfNanobots; ++i)
    {
      group = (i - 1) / intervall;
      Ptr<Nanobot> temp_nb = CreateObject<Nanobot> ();
      temp_nb->SetNanobotID (i);
      //Get random stream number.
      int dr = floor (distribute_randomly->GetValue ());
      temp_nb->SetShouldChange (false);
      temp_nb->SetPosition (Vector (m_coordinateStart.x + (directionIntervall.x * group),
                                    m_coordinateStart.y + (directionIntervall.y * group),
                                    m_coordinateStart.z + (directionIntervall.z * group)));
      //Set position with random stream dr.
      bloodvessel->AddNanobotToStream (dr, temp_nb);
    }
  //Print Nanobots in csv-file.

  bloodvessel->PrintNanobotsOfVessel ();
}

double
Bloodcircuit::GetSpeedClassOfBloodVesselType (BloodvesselType type)
{
  if (type == ARTERY)
    {
      return 10.0;
    }
  else if (type == VEIN)
    {
      return 3.7;
    }
  else // if (type == ORGAN)
    {
      return 1.0;
    }
}

void
Bloodcircuit::AddVesselData (int id, BloodvesselType type, Vector start, Vector stop)
{
  Ptr<Bloodvessel> vessel = CreateObject<Bloodvessel> ();
  vessel->SetBloodvesselID (id);
  vessel->SetBloodvesselType (type);
  vessel->SetStartPositionBloodvessel (start);
  vessel->SetStopPositionBloodvessel (stop);
  vessel->SetVesselWidth (0.25); // 0.25
  vessel->SetPrinter (printer);
  // Init Bloodvessel: Calculate length and angle & velocity.
  vessel->InitBloodstreamLengthAngleAndVelocity (GetSpeedClassOfBloodVesselType (type));
  m_bloodvessels[id] = vessel;
  cout << "New Vessel(" + to_string (id) + "," + to_string (type) + "," + to_string (start.x) +
              "," + to_string (start.y) + "," + to_string (start.z) + "," + to_string (stop.x) +
              "," + to_string (stop.y) + "," + to_string (stop.z) + ")"
       << endl;
}

void
Bloodcircuit::InitialiseBloodvessels (int vesseldata[][8], unsigned int count)
{
  for (unsigned int i = 0; i < count; i++)
    {
      AddVesselData (vesseldata[i][0], (BloodvesselType) vesseldata[i][1],
                     Vector (vesseldata[i][2], vesseldata[i][3], vesseldata[i][4]),
                     Vector (vesseldata[i][5], vesseldata[i][6], vesseldata[i][7]));
    }
}

void
Bloodcircuit::ConnectBloodvessels ()
{
  unsigned int count = m_bloodvessels.size();
  //Set Connections between bloodvessels if they have the same start/end coordinates
  for (unsigned int i = 1; i < count + 1; i++)
    {
      unsigned int counter = 0;
      Vector end = m_bloodvessels[i]->GetStopPositionBloodvessel ();
      for (unsigned int j = 1; j < count + 1; j++)
        {
          Vector start = m_bloodvessels[j]->GetStartPositionBloodvessel ();
          if (end.x == start.x && end.y == start.y && end.z == start.z)
            {
              counter++;
              if (counter == 1)
                {
                  m_bloodvessels[i]->SetNextBloodvessel1 (m_bloodvessels[j]);
                }
              else
                {
                  m_bloodvessels[i]->SetNextBloodvessel2 (m_bloodvessels[j]);
                }
            }
        }
    }
}

} // namespace ns3
