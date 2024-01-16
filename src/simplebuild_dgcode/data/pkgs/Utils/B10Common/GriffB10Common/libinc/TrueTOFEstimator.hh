#ifndef GriffB10Common_TrueTOFEstimator_hh
#define GriffB10Common_TrueTOFEstimator_hh

#include "GriffDataRead/GriffDataReader.hh"

namespace GriffB10Common {

  //For events where the primary particle was a neutron + a codirectional
  //geantinoe (added via the G4GeantinoInserter module), the following function
  //attempts to estimate the arrival time that the neutron would have on a given
  //volume (default "Converter"), if it would not have had any interactions on
  //its way there. The calculations estimates the time of flight by combining:
  //
  //  * The pathlength of the geantino to the first volume with the given name.
  //  * The initial velocity of the neutron.
  //
  //The returned time is added to the initial time of the neutron (for many
  //generators this is 0).
  //
  //The function returns false when the time of flight could not be
  //calculated. This happens if the event does not contain exactly two primary
  //particles which are respectively a neutron & a geantino, if the path of the
  //geantino never intersects a volume of the given name, or if the Griff event
  //is in MINIMAL mode and thus lacks step coordinate information.

  bool estimateTrueTOF( GriffDataReader& dr,
                        double& true_tof,
                        const std::string& volname = "Converter" );

  //Version which also requires a match of the volume copy number:
  bool estimateTrueTOF( GriffDataReader& dr,
                        double& true_tof,
                        int volcopynbr,
                        const std::string& volname = "Converter" );
}

#endif
