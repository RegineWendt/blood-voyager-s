#include "Bloodcircuit.h"

using namespace ns3;
using namespace std;

class BloodCircuitTestCase : public TestCase
{
public:
  BloodCircuitTestCase ();

private:
  virtual void DoRun (void);
};

BloodCircuitTestCase::BloodCircuitTestCase () : TestCase ("Check some bit of functionality")
{
}

void
BloodCircuitTestCase::DoRun (void)
{
  //Add default values:
  int numOfNanobots = 300;
  int simulationDuration = 300;
  int injectionVessel = 65;

  NS_TEST_ASSERT_MSG_EQ (Bloodcircuit::BeginnSimulation (simulationDuration, numOfNanobots, injectionVessel), 0, "Wrong Exit Code");
}

class BloodVoyagerSTestSuite : public TestSuite
{
public:
  BloodVoyagerSTestSuite ();
};

BloodVoyagerSTestSuite::BloodVoyagerSTestSuite () : TestSuite ("blood-voyager-s", UNIT)
{
  AddTestCase (new BloodCircuitTestCase, TestCase::QUICK);
}

static BloodVoyagerSTestSuite bloodVoyagerSTestSuite;