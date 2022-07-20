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

#ifndef CLASS_BLOODVESSEL_
#define CLASS_BLOODVESSEL_

#include "PrintNanobots.h"
#include "Bloodstream.h"

using namespace std;
namespace ns3 {
/**
* \brief Bloodvessel is the place holder of the Nanobot's and manages each step of the Nanobot's mobility.
*
* A Bloodvessel has up to total 5 lists (maximum 5 streams). At each step (interval dt), Bloodvessel
* browses the nanobots of each stream in order of their positions and moves each Nanobot. If the resulting position exceeds the current bloodvessel
* the nanobot gets pushed to the next bloodvessel (and so on).
* Nanobots are added to the Bloodvessels in Bloodcircuit.
*/
enum BloodvesselType { ARTERY, VEIN, ORGAN };
class Bloodvessel : public ns3::Object
{
private:
  bool m_start;
  vector<Ptr<Bloodstream>> m_bloodstreams; // list of nanobots in streams.
  int m_bloodvesselID; // unique ID, set in bloodcircuit.
  double m_bloodvesselLength; // the length of the bloodvessel.
  double m_angle; // the angle of the bloodvessel.
  double m_basevelocity; // velocity of the bloodvessel.
  BloodvesselType m_bloodvesselType; // type of the bloodvessel: 0=artery, 1=vein, 2=organ
  Vector m_startPositionBloodvessel; // start-coordinates of the bloodvessel
  Vector m_stopPositionBloodvessel; // end-coordinates of the bloodvessel
  double m_deltaT; // the mobility step interval (duration between each step).
  double m_vesselWidth; // the width of each stream in the bloodvessel.
  Ptr<PrintNanobots> printer;

  /// Connections
  Ptr<Bloodvessel> m_nextBloodvessel1;
  Ptr<Bloodvessel> m_nextBloodvessel2;

  /// Stream settings
  int m_numberOfStreams; // number of streams, maximum value is 5.

  bool m_changeStreamSet; // true, if the nanobots should be able to change their streams.
  int m_currentStream; // current stream number.

  /// Output file with positions and timesteps.
  ofstream m_nbTrace;
  string m_nbTraceFilename;

  const int stream_definition[21][3] = {
      {100, 0, 0},  {95, -1, 0},  {95, +1, 0},  {95, 0, -1},  {95, 0, +1},  {95, -1, -1},
      {95, +1, +1}, {95, +1, -1}, {95, -1, +1}, {90, +2, 0},  {90, -2, 0},  {90, 0, +2},
      {90, 0, -2},  {90, +2, -1}, {90, -2, +1}, {90, -1, +2}, {90, +1, -2}, {90, +2, +1},
      {90, -2, -1}, {90, +1, +2}, {90, -1, -2}};
  const int stream_definition_size = 21;

  Vector SetPosition (Vector nbv, double distance, double angle, int bloodvesselType,
                      double startPosZ);

  // calculate Angle
  double CalcAngle ();

  // calculate Length
  double CalcLength ();

  /**
  * \param value the Number of Streams the bloodvessel can have.
  */
  void initStreams ();

  /// Translates the Nanobots to the new position. Calles TranslatePosition (), ChangeStream (), TransposeNanobots ().
  void TranslateNanobots ();

  /// Calculates the position and velocity of each nanobot for the passed step and the next step.
  void TranslatePosition (double dt);

  /// Changes the nanobot streams if possible. Calles DoChangeStreamIfPossible ().
  void ChangeStream ();

  /// Changes the nanobot streams from current streams to the destination stream.
  void DoChangeStreamIfPossible (int curStream, int desStream);

  /// Transposes Nanobots from one bloodvessel to another.
  void TransposeNanobots (list<Ptr<Nanobot>> reachedEnd, int i);

  /// Moves one Nanobot to the next bloodvessel
  bool transposeNanobot (Ptr<Nanobot> botToTranspose, Ptr<Bloodvessel> thisBloodvessel,
                         Ptr<Bloodvessel> nextBloodvessel, int stream);
  /**
   * /return TRUE if position exceeds bloodvessel
   */
  bool moveNanobot (Ptr<Nanobot> nb, int i, int randVelocityOffset, bool direction, double dt);

  //GETTER AND SETTER

  /**
  * \returns true, if all streams of the bloodvessel are empty.
  */
  bool IsEmpty ();

  /**
  * \returns the Type of the Bloodvessel.
  */
  BloodvesselType GetBloodvesselType ();

  /**
  * \returns the Angle of the Bloodvessel.
  */
  double GetBloodvesselAngle ();

  /**
  * \returns the Length of the Bloodvessel.
  */
  double GetbloodvesselLength ();

  bool getRandomBoolean ();

public:
  /**
  * Setting the default values:
  * dt=1.0, number of streams=3, changing stream set to true, velocity and current stream is zero.
  * x-, y- and z-direction are empty.
  */
  Bloodvessel ();

  /// Destructor to clean up the lists.
  ~Bloodvessel ();

  /**
  * Starts the bloodvessel.
  */
  void Start ();

  /**
  * Runs one mobility Step for the given bloodvessel.
  * This function is called each interval dt to simulate the mobility through TranslateNanobots ().
  */
  static void Step (Ptr<Bloodvessel> bloodvessel);

  /// Prints all nanobots in the Bloodvessel to a csv file.
  void PrintNanobotsOfVessel ();

  /**
  * \param value the traffic velocity m/s at entrance.
  */
  void InitBloodstreamLengthAngleAndVelocity (double velocity);

  /**
  * \param streamID: ID of Stream
  * \param bot: Pointer to bot to add 
  */
  void AddNanobotToStream (unsigned int streamID, Ptr<Nanobot> bot);

  //GETTER AND SETTER

  /**
  * overrides TypeId.
  */
  static TypeId GetTypeId (void);

  /**
  * \param Id of a Stream 
  * \returns a specific stream
  */
  Ptr<Bloodstream> GetStream (int id);

  /**
  * \returns the ID of the Bloodvessel.
  */
  int GetbloodvesselID ();

  /**
  * \returns the Number of Streams in the Bloodvessel.
  */
  int GetNumberOfStreams ();

  /**
  * \returns the Startposition of the Bloodvessel.
  */
  Vector GetStartPositionBloodvessel ();

  /**
  * \returns the stopposition of the Bloodvessel.
  */
  Vector GetStopPositionBloodvessel ();

  /**
   * gets an Random Object
   */
  Ptr<UniformRandomVariable> getRandomObjectBetween (double min, double max);

  /**
  * \param value the ID of the Bloodvessel.
  */
  void SetBloodvesselID (int b_id);

  /**
  * \param value the type of the Bloodvessel.
  */
  void SetBloodvesselType (BloodvesselType value);

  /**
  * \param value the Width of the Streams.
  */
  void SetVesselWidth (double value);

  /**
  * \param value the start position of the Bloodvessel.
  */
  void SetStartPositionBloodvessel (Vector value);

  /**
  * \param value the start position of the Bloodvessel.
  */
  void SetStopPositionBloodvessel (Vector value);

  /**
  * \param value the following Bloodvessel.
  */
  void SetNextBloodvessel1 (Ptr<Bloodvessel> value);

  /**
  * \param value the following Bloodvessel.
  */
  void SetNextBloodvessel2 (Ptr<Bloodvessel> value);

  void SetPrinter (Ptr<PrintNanobots> printer);
};
}; // namespace ns3
#endif
