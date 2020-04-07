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
 * Author: Regine Wendt <wendt@itm.uni-luebeck.de>, Chris Deter
 */

#ifndef CLASS_PRINTNANOBOT_
#define CLASS_PRINTNANOBOT_

#include "Bloodstream.h"

using namespace std;
namespace ns3 {
/**
* \brief Short Description
*
* Long Description
*/
class PrintNanobots : public ns3::Object
{
private:
  ofstream output;

public:
  PrintNanobots ();

  ~PrintNanobots ();

  /// Prints one nanobot to a csv file.
  void PrintNanobot (Ptr<Nanobot> n, int vesselID);

  /// Prints transposed/translated nanobots in the Bloodvessel to a csv file.
  void PrintSomeNanobots (list<Ptr<Nanobot>> nbl, int vesselID);

  // Debug Function, currently not used
  void PrintInTerminal (vector<Ptr<Bloodstream>> streamsOfVessel, int vesselIDl);

}; //  Class End
}; // namespace ns3
#endif
