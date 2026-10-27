#include "sbldpy/sbldpy.hh"

#include "CLHEP/Units/SystemOfUnits.h"
#include "G4UniformGravityField.hh"
#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"
#include "G4RepleteEofM.hh"
#include "G4EqGravityField.hh"
#include "G4ClassicalRK4.hh"
#include "G4MagIntegratorStepper.hh"
#include "G4ChordFinder.hh"
#include "G4PropagatorInField.hh"

class NeutronGravity{
public:
  NeutronGravity()
  :m_x_dir(0.), m_y_dir(-1.), m_z_dir(0.), m_g(9.80665)
  {
  };
  ~NeutronGravity(){};

  void setDir(double x, double y, double z)
  {
    m_x_dir=x;
    m_y_dir=y;
    m_z_dir=z;
  }

  void set()
  {
    G4ThreeVector fieldDir (m_x_dir,m_y_dir,m_z_dir);

//    if( fabs(fieldDir.mag()-1.) > 1e-4 )
//    {
//      std::stringstream str;
//      str<<m_x_dir<< " " << m_y_dir << " " << m_z_dir;
//      G4Exception ("NeutronGravity::set", "input is not a unit vector",
//          FatalErrorInArgument, str.str().c_str());
//    }
    fieldDir=fieldDir.unit();

    G4UniformGravityField *fField = new G4UniformGravityField(fieldDir*m_g*CLHEP::m/CLHEP::s/CLHEP::s);

    G4EqGravityField* equation = new G4EqGravityField(fField);

    G4FieldManager* fieldManager
    = G4TransportationManager::GetTransportationManager()->GetFieldManager();
    fieldManager->SetDetectorField(fField);

    G4MagIntegratorStepper* stepper = new G4ClassicalRK4(equation,8);

    G4double minStep           = 0.01*CLHEP::mm;

    G4ChordFinder* chordFinder =
        new G4ChordFinder((G4MagneticField*)fField,minStep,stepper);

    // parameters taken from the G4 example "filed6"
    G4double deltaChord        = 3.0*CLHEP::mm;
    chordFinder->SetDeltaChord( deltaChord );

    G4double deltaOneStep      = 0.01*CLHEP::mm;
    fieldManager->SetAccuraciesWithDeltaOneStep(deltaOneStep);

    G4double deltaIntersection = 0.1*CLHEP::mm;
    fieldManager->SetDeltaIntersection(deltaIntersection);

    G4TransportationManager* transportManager =
        G4TransportationManager::GetTransportationManager();

    G4PropagatorInField* fieldPropagator =
        transportManager->GetPropagatorInField();

    G4double epsMin            = 2.5e-7*CLHEP::mm;
    G4double epsMax            = 0.005*CLHEP::mm;

    fieldPropagator->SetMinimumEpsilonStep(epsMin);
    fieldPropagator->SetMaximumEpsilonStep(epsMax);

    fieldManager->SetChordFinder(chordFinder);

  }
private:
  double m_x_dir, m_y_dir, m_z_dir, m_g;
};



SBLDPY_MODULE( hooks )
{
  m.add_class<NeutronGravity>("NeutronGravity")
    .def_init<>()
    .def("setDir", &NeutronGravity::setDir, { "x", "y", "z" })
    .def("set", &NeutronGravity::set);
}
