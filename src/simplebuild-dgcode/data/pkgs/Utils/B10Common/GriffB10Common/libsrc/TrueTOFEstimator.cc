#include "GriffB10Common/TrueTOFEstimator.hh"
#include "GriffB10Common/Utils.hh"
#include "Utils/ArrayMath.hh"
#include "Utils/Kinematics.hh"
#include <cassert>
#include <limits>

namespace GriffB10Common {

  bool estimateTrueTOFImpl( GriffDataReader& dr,
                            double& true_tof,
                            const std::string& volname,
                            int *copynbr)
  {
    if (dr.eventStorageMode()==GriffFormat::Format::MODE_MINIMAL)
      return false;

    const GriffDataRead::Track* n;
    const GriffDataRead::Track* g;
    if (!getPrimaryNeutronAndGeantino(dr,n,g))
      return false;

    const double* p;
    if (copynbr)
      p = firstVolIntersectionPos(g,volname,*copynbr);
    else
      p = firstVolIntersectionPos(g,volname);
    if (!p)
      return false;

    auto n0 = n->firstStep();
    const double l = Utils::dist(p,n0->preGlobalArray());

    double v = Utils::velocity(n0->preEKin(), n->mass());
    true_tof = n->firstStep()->preTime() + ( v > 0.0 ? l/v : std::numeric_limits<double>::infinity() );
    return true;
  }



  bool estimateTrueTOF( GriffDataReader& dr,
                        double& true_tof,
                        int volcopynbr,
                        const std::string& volname )
  {
    return estimateTrueTOFImpl(dr,true_tof,volname,&volcopynbr);
  }

  bool estimateTrueTOF( GriffDataReader& dr,
                        double& true_tof,
                        const std::string& volname )
  {
    return estimateTrueTOFImpl(dr,true_tof,volname,0);
  }

}
