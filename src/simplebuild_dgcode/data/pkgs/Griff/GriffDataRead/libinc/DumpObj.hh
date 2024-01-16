#ifndef GriffDataRead_DumpObj_hh
#define GriffDataRead_DumpObj_hh

namespace GriffDataRead {

  class Track;
  class Segment;
  class Step;
  class Material;
  class Element;
  class Isotope;

  //Dumps *all* object data on stdout without newline character. Does not dump
  //to referenced objects (like parent/daughter tracks or contained segments and
  //steps). This is not particularly readable but can be useful for debugging in
  //addition to tests:

  void dump(const Track*, bool dumpPDGInfo = false );
  void dump(const Segment*);
  void dump(const Step*);
  void dump(const Material*);
  void dump(const Element*);
  void dump(const Isotope*);

  inline void dump(const Track& trk, bool dumpPDGInfo = false ) { dump(&trk,dumpPDGInfo); }
  inline void dump(const Segment& segment) { dump(&segment); }
  inline void dump(const Step& step) { dump(&step); }
  inline void dump(const Material& mat) { dump(&mat); }
}

#endif
