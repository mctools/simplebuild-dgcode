#ifndef GriffAnaUtils_TrackFilter_PDGCode_hh
#define GriffAnaUtils_TrackFilter_PDGCode_hh

#include "GriffAnaUtils/ITrackFilter.hh"
#include "Utils/FastLookupSet.hh"
#include "Core/Types.hh"
#include <set>

namespace GriffAnaUtils {

  class TrackFilter_PDGCode : public ITrackFilter {
  public:
    //For convenience, up to 5 pdg codes can be specified directly in the
    //constructor. For more, please use the addCode or addCodes calls.
    TrackFilter_PDGCode();
    TrackFilter_PDGCode(int32_t c1);
    TrackFilter_PDGCode(int32_t c1,int32_t c2);
    TrackFilter_PDGCode(int32_t c1,int32_t c2,int32_t c3);
    TrackFilter_PDGCode(int32_t c1,int32_t c2,int32_t c3,int32_t c4);
    TrackFilter_PDGCode(int32_t c1,int32_t c2,int32_t c3,int32_t c4,int32_t c5);

    //The filter can be either "signed" (default) or "unsigned", meaning whether
    //or not the pdgcode comparisons consider the sign. Example 11(e-) and
    //-11(e+). When the filter is unsigned both will match pdgcode 11, when the
    //filter is signed, only e- will be matched.

    //For conveniency, setters/adders return a pointer to the same object:
    TrackFilter_PDGCode * addCode(int32_t c);
    TrackFilter_PDGCode * addCodes(int32_t c);
    TrackFilter_PDGCode * addCodes(int32_t c1,int32_t c2);
    TrackFilter_PDGCode * addCodes(int32_t c1,int32_t c2,int32_t c3);
    TrackFilter_PDGCode * addCodes(int32_t c1,int32_t c2,int32_t c3,int32_t c4);
    TrackFilter_PDGCode * addCodes(int32_t c1,int32_t c2,int32_t c3,int32_t c4,int32_t c5);
    TrackFilter_PDGCode * setSigned();
    TrackFilter_PDGCode * setUnsigned();

    bool isSigned();
    bool isUnsigned();

    bool filter(const GriffDataRead::Track*trk) const;

  private:
    TrackFilter_PDGCode( const TrackFilter_PDGCode & );
    TrackFilter_PDGCode & operator= ( const TrackFilter_PDGCode & );
    virtual ~TrackFilter_PDGCode(){}
    bool m_unsigned;
#if 1//fixme, once we trust these sets we should use Utils::FastLookupSet always
    Utils::FastLookupSet<int32_t> m_allAbsCodes;
    Utils::FastLookupSet<int32_t> m_positiveCodes;//includes "0"
    Utils::FastLookupSet<int32_t> m_negativeCodes;
#else
    std::set<int32_t> m_allAbsCodes;
    std::set<int32_t> m_positiveCodes;//includes "0"
    std::set<int32_t> m_negativeCodes;
#endif
  };
}

#include "GriffAnaUtils/TrackFilter_PDGCode.icc"

#endif
