namespace GDR_segment {
  namespace {
    using Segment = GriffDataRead::Segment;
    using Material = GriffDataRead::Material;

    //default arguments are a bit annoying (fixme: can be done nicer in pybind now)
    int pycpp_volumeCopyNumber_0args(const Segment*s) { return s->volumeCopyNumber(); }
    const char * pycpp_physicalVolumeNameCStr_0args(const Segment*s) { return s->physicalVolumeNameCStr(); }
    const char * pycpp_volumeNameCStr_0args(const Segment*s) { return s->volumeNameCStr(); }

    //Due to lack of overloading, we add dump methods directly to the objects in python:
    void dump_segment(Segment*s) { dump(s); }
    const Material* segment_material_0args(Segment*s) { return s->material(); }

    void pyexport( pybind11::module_ themod )
    {
      py::class_<Segment,std::unique_ptr<Segment, BlankDeleter<Segment>> >(themod,"Segment")
        .def("iSegment",&Segment::iSegment)
        .def("nStepsOriginal",&Segment::nStepsOriginal)
        .def("nStepsStored",&Segment::nStepsStored)
        .def("segmentLength",&Segment::segmentLength)
        .def("startTime",&Segment::startTime)
        .def("endTime",&Segment::endTime)
        .def("startEKin",&Segment::startEKin)
        .def("endEKin",&Segment::endEKin)
        .def("eDep",&Segment::eDep)
        .def("eDepNonIonising",&Segment::eDepNonIonising)
        .def("startAtVolumeBoundary",&Segment::startAtVolumeBoundary)
        .def("endAtVolumeBoundary",&Segment::endAtVolumeBoundary)
        .def("volumeDepthStored",&Segment::volumeDepthStored)
        .def("volumeName",&Segment::volumeNameCStr)
        .def("volumeName",&pycpp_volumeNameCStr_0args)
        .def("physicalVolumeName",&Segment::physicalVolumeNameCStr)
        .def("physicalVolumeName",&pycpp_physicalVolumeNameCStr_0args)
        .def("material",&Segment::material,py::return_value_policy::reference)
        .def("material",&segment_material_0args,py::return_value_policy::reference)
        .def("volumeCopyNumber",&Segment::volumeCopyNumber)
        .def("volumeCopyNumber",&pycpp_volumeCopyNumber_0args)
        .def("isInWorldVolume",&Segment::isInWorldVolume)
        .def("getTrack",&Segment::getTrack,py::return_value_policy::reference)
        .def("getNextSegment",&Segment::getNextSegment,py::return_value_policy::reference)
        .def("getPreviousSegment",&Segment::getPreviousSegment,py::return_value_policy::reference)
        .def("nextWasFiltered",&Segment::nextWasFiltered)
        .def("getStep",&Segment::getStep,py::return_value_policy::reference)
        .def("hasStepInfo",&Segment::hasStepInfo)
        .def("stepBegin",&Segment::stepBegin,py::return_value_policy::reference)
        .def("stepEnd",&Segment::stepEnd,py::return_value_policy::reference)
        .def("firstStep",&Segment::firstStep,py::return_value_policy::reference)
        .def("lastStep",&Segment::lastStep,py::return_value_policy::reference)
        .def("dump",&dump_segment)
        .def("inSameVolume",&Segment::inSameVolume)
        ;
    }
  }
}
