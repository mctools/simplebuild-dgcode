#ifndef GriffFormat_Format_hh
#define GriffFormat_Format_hh

#include "Core/Types.hh"
#include "EvtFile/IFormat.hh"
#include "EvtFile/Defs.hh"

namespace GriffFormat {

  class Format : public EvtFile::IFormat
  {
  public:

    static const Format * getFormat();

    virtual std::uint32_t magicWord() const { return 0xe5506ea4; } //~= ess_gea4
    virtual const char* fileExtension() const { return ".griff"; }
    virtual const char* eventBriefDataName() const { return "track"; }
    virtual const char* eventFullDataName() const { return "step"; }

    virtual bool compressFullData() const { return true; }

    static const EvtFile::subsectid_type subsectid_touchables = 100;
    static const EvtFile::subsectid_type subsectid_volnames = 110;
    static const EvtFile::subsectid_type subsectid_materials = 120;
    static const EvtFile::subsectid_type subsectid_elements = 130;
    static const EvtFile::subsectid_type subsectid_isotopes = 140;
    static const EvtFile::subsectid_type subsectid_materialnames = 150;
    static const EvtFile::subsectid_type subsectid_elementnames = 160;
    static const EvtFile::subsectid_type subsectid_isotopenames = 170;
    static const EvtFile::subsectid_type subsectid_procnames = 200;
    static const EvtFile::subsectid_type subsectid_pdgcodes = 300;
    static const EvtFile::subsectid_type subsectid_pdgnames = 310;
    static const EvtFile::subsectid_type subsectid_pdgtypes = 320;
    static const EvtFile::subsectid_type subsectid_pdgsubtypes = 330;
    static const EvtFile::subsectid_type subsectid_metadata = 400;
    static const EvtFile::subsectid_type subsectid_metadatastrings = 410;

    enum MODE { MODE_FULL=0, MODE_REDUCED=1, MODE_MINIMAL=3 };

    //Three modes:
    //1) full => store all step data
    //2) coalesce adjacent steps in the same volume(/filtered sequence) => all segments will have just 1 step
    //3) minimal => no step info at all, just tracks and segments.

    //For the implementation of file writer/reader we provide a common reference of expected sizes:
    static const unsigned SIZE_TRACKHEADER = sizeof(std::uint32_t)*2+sizeof(std::uint64_t)+sizeof(EvtFile::index_type);
    static const unsigned SIZE_PER_TRACK_WO_DAUGHTERLIST = sizeof(std::uint32_t)*5+sizeof(float)+sizeof(EvtFile::index_type);
    static const unsigned SIZE_PER_DAUGHTERLIST_ENTRY = sizeof(std::uint32_t);
    static const unsigned SIZE_PER_SEGMENT = sizeof(double)*2+sizeof(float)*2+sizeof(EvtFile::index_type)*2;
    static const unsigned SIZE_LAST_SEGMENT_ON_TRACK_EXTRA_SIZE = sizeof(double)*2;

    static const unsigned SIZE_STEPHEADER = 2*sizeof(std::uint32_t);
    static const unsigned SIZE_STEPPREPOSTPART = 68;
    static const unsigned SIZE_STEPOTHERPART = 3*sizeof(float)+sizeof(std::uint32_t);

  private:
    Format(){}
    virtual ~Format() {}
  };

}

#endif
