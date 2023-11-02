#ifndef GriffAnaUtils_ITrackFilter_hh
#define GriffAnaUtils_ITrackFilter_hh

#include "GriffAnaUtils/IFilter.hh"
#include "GriffDataRead/Track.hh"

namespace GriffAnaUtils {

  class ITrackFilter : public IFilter {
  public:
    //NOTE: Destructor of derived classes must not be public!
    virtual bool filter(const GriffDataRead::Track*) const = 0;
  protected:
    ITrackFilter(){}
    virtual ~ITrackFilter(){}
  private:
    ITrackFilter( const ITrackFilter & );
    ITrackFilter & operator= ( const ITrackFilter & );
  };
}

#endif
