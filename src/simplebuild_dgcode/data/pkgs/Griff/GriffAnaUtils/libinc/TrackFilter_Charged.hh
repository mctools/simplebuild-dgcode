#ifndef GriffAnaUtils_TrackFilter_Charged_hh
#define GriffAnaUtils_TrackFilter_Charged_hh

#include "GriffAnaUtils/ITrackFilter.hh"

//Picks charged particles (q!=0) only.

namespace GriffAnaUtils {

  class TrackFilter_Charged : public ITrackFilter {
  public:
    TrackFilter_Charged();

    TrackFilter_Charged * setNegativeOnly();//require q<0
    TrackFilter_Charged * setPositiveOnly();//require q>0

    bool filter(const GriffDataRead::Track*trk) const;

  private:
    TrackFilter_Charged( const TrackFilter_Charged & );
    TrackFilter_Charged & operator= ( const TrackFilter_Charged & );
    virtual ~TrackFilter_Charged(){}
    bool m_allowNegative;
    bool m_allowPositive;
  };
}

#include "GriffAnaUtils/TrackFilter_Charged.icc"

#endif
