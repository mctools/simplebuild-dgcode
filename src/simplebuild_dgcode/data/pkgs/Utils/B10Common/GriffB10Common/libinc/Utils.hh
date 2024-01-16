#ifndef GriffB10Common_Utils_hh
#define GriffB10Common_Utils_hh

#include "GriffDataRead/GriffDataReader.hh"

namespace GriffB10Common {

  //Returns true (and sets trk_neutron+trk_geantino) if and only if the event
  //has exactly two primary particle, a neutron and a geantino. This is useful
  //for analysing neutron events augmented with the G4GeantinoInserter module.

  bool getPrimaryNeutronAndGeantino(GriffDataReader& dr,
                                    const GriffDataRead::Track*& trk_neutron,
                                    const GriffDataRead::Track*& trk_geantino);

  //Returns the coordinates (pointer to double[3]) of where the track first
  //passes into a volume of a given name. Returns null-pointer in case the track
  //never intersects such a volume:
  const double* firstVolIntersectionPos( const GriffDataRead::Track*trk,
                                         const std::string& volname);

  //Version which also requires a match of the volume copy number:
  const double* firstVolIntersectionPos( const GriffDataRead::Track*trk,
                                         const std::string& volname,
                                         int volcopynbr);

}

#endif
