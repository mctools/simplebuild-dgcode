namespace GDR_track {
  namespace {
    using Track = GriffDataRead::Track;

    //Due to lack of overloading, we add dump methods directly to the objects in python:
    void dump_track(Track*trk) { dump(trk,false); }
    void dump_track_withpdginfo(Track*trk) { dump(trk,true); }

    void pyexport( pybind11::module_ themod )
    {
      py::class_<Track,std::unique_ptr<Track, BlankDeleter<Track>> >(themod,"Track")
        .def("trackID",&Track::trackID)
        .def("parentID",&Track::parentID)
        .def("nDaughters",&Track::nDaughters)
        .def("nSegments",&Track::nSegments)
        .def("weight",&Track::weight)
        .def("isPrimary",&Track::isPrimary)
        .def("isSecondary",&Track::isSecondary)
        .def("startTime",&Track::startTime)
        .def("startEKin",&Track::startEKin)
        .def("pdgCode",&Track::pdgCode)
        .def("pdgName",&Track::pdgNameCStr)
        .def("pdgType",&Track::pdgTypeCStr)
        .def("pdgSubType",&Track::pdgSubTypeCStr)
        .def("creatorProcess",&Track::creatorProcessCStr)
        .def("mass",&Track::mass)
        .def("width",&Track::width)
        .def("charge",&Track::charge)
        .def("lifeTime",&Track::lifeTime)
        .def("atomicNumber",&Track::atomicNumber)
        .def("atomicMass",&Track::atomicMass)
        .def("magneticMoment",&Track::magneticMoment)
        .def("spin",&Track::spin)
        .def("stable",&Track::stable)
        .def("shortLived",&Track::shortLived)
        .def("getDaughterID",&Track::getDaughterID)
        .def("getParent",&Track::getParent,py::return_value_policy::reference)
        .def("getDaughter",&Track::getDaughter,py::return_value_policy::reference)
        .def("getSegment",&Track::getSegment,py::return_value_policy::reference)
        .def("segmentBegin",&Track::segmentBegin,py::return_value_policy::reference)
        .def("segmentEnd",&Track::segmentEnd,py::return_value_policy::reference)
        .def("firstSegment",&Track::firstSegment,py::return_value_policy::reference)
        .def("lastSegment",&Track::lastSegment,py::return_value_policy::reference)
        .def("firstStep",&Track::firstStep,py::return_value_policy::reference)
        .def("lastStep",&Track::lastStep,py::return_value_policy::reference)
        .def("dump",&dump_track)
        .def("dump_full",&dump_track_withpdginfo)
        ;
    }
  }
}
