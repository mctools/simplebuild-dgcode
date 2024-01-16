#include "Core/Python.hh"

#include "GriffDataRead/GriffDataReader.hh"
#include "GriffDataRead/Track.hh"
#include "GriffDataRead/Segment.hh"
#include "GriffDataRead/Step.hh"
#include "GriffDataRead/DumpObj.hh"
#include "GriffDataRead/Material.hh"
#include "GriffDataRead/Element.hh"
#include "GriffDataRead/Isotope.hh"

namespace {
  template < typename T>
  struct BlankDeleter
  {
    void operator()(T *) const {}
  };
}

#include "step.hh"
#include "segment.hh"
#include "track.hh"
#include "materials.hh"

namespace {
  py::dict pyGriffDataReadSetup_metaData(GriffDataRead::Setup*self)
  {
    py::dict d;
    const auto& m = self->metaData();
    auto itE = m.end();
    for (auto it=m.begin();it!=itE;++it) {
      d[it->first.c_str()]=it->second.c_str();
    }
    return d;
  }

  py::dict pyGriffDataReadSetup_userData(GriffDataRead::Setup*self)
  {
    py::dict d;
    const auto& m = self->userData();
    auto itE = m.end();
    for (auto it=m.begin();it!=itE;++it)
      d[it->first.c_str()]=it->second.c_str();
    return d;
  }

  py::list pyGriffDataReadSetup_cmds(GriffDataRead::Setup*self)
  {
    py::list l;
    const auto& cmds = self->cmds();
    auto itE = cmds.end();
    for (auto it=cmds.begin();it!=itE;++it)
      l.append(*it);
    return l;
  }

  void pyGriffDataReadSetup_dump_0args(GriffDataRead::Setup*self) { self->dump(); }
  void pyGriffDataRead_GeoParams_dump_0args(GriffDataRead::GeoParams*self) { self->dump(); }
  void pyGriffDataRead_GenParams_dump_0args(GriffDataRead::GenParams*self) { self->dump(); }
  void pyGriffDataRead_FilterParams_dump_0args(GriffDataRead::FilterParams*self) { self->dump(); }
}

PYTHON_MODULE( mod )
{
  pyextra::pyimport("Utils.DummyParamHolder");
  pyextra::pyimport("Utils.RefCountBase");

  GDR_step::pyexport(mod);
  GDR_segment::pyexport(mod);
  GDR_track::pyexport(mod);
  GDR_material::pyexport(mod);
  py::class_<GriffDataRead::GeoParams,
             std::unique_ptr<GriffDataRead::GeoParams, BlankDeleter<GriffDataRead::GeoParams>>,
             Utils::DummyParamHolder>(mod,"GeoParams")
    .def("getName",&GriffDataRead::GeoParams::getNameCStr)
    .def("dump",&GriffDataRead::GeoParams::dump)
    .def("dump",&pyGriffDataRead_GeoParams_dump_0args)
    ;

  py::class_<GriffDataRead::GenParams,
             std::unique_ptr<GriffDataRead::GenParams, BlankDeleter<GriffDataRead::GenParams>>,
             Utils::DummyParamHolder>(mod,"GenParams")
    .def("getName",&GriffDataRead::GenParams::getNameCStr)
    .def("dump",&GriffDataRead::GenParams::dump)
    .def("dump",&pyGriffDataRead_GenParams_dump_0args)
    ;

  py::class_<GriffDataRead::FilterParams,
             std::unique_ptr<GriffDataRead::FilterParams, BlankDeleter<GriffDataRead::FilterParams>>,
             Utils::DummyParamHolder>(mod,"FilterParams")
    .def("getName",&GriffDataRead::FilterParams::getNameCStr)
    .def("dump",&GriffDataRead::FilterParams::dump)
    .def("dump",&pyGriffDataRead_FilterParams_dump_0args)
    ;

  py::class_<GriffDataRead::Setup,
             std::unique_ptr<GriffDataRead::Setup, BlankDeleter<GriffDataRead::Setup>>,
             Utils::RefCountBase>(mod,"Setup")
    .def("metaData",&pyGriffDataReadSetup_metaData)
    .def("userData",&pyGriffDataReadSetup_userData)
    .def("geo",&GriffDataRead::Setup::geo,py::return_value_policy::reference)
    .def("gen",&GriffDataRead::Setup::gen,py::return_value_policy::reference)
    .def("hasFilter",&GriffDataRead::Setup::hasFilter)
    //    .def("filter",&GriffDataRead::Setup::filter,py::return_value_policy::reference)
    .def("getFilter",&GriffDataRead::Setup::filter,py::return_value_policy::reference)//Different name on py-side to avoid clash with inbuilt
    .def("cmds",&pyGriffDataReadSetup_cmds)
    .def("dump",&GriffDataRead::Setup::dump)
    .def("dump",&pyGriffDataReadSetup_dump_0args)
    ;

  py::class_<GriffDataReader,std::shared_ptr<GriffDataReader>>(mod,"GriffDataReader")
    .def(py::init<std::string>())
    .def(py::init( []( py::list l )
    {
      const std::size_t n = static_cast<std::size_t>(py::len(l));
      std::vector<std::string> v;
      v.reserve(n);
      for (std::size_t i = 0; i < n; ++i)
        v.push_back( l[i].cast<std::string>() );
      return std::make_shared<GriffDataReader>(v);
    }))
    .def(py::init( []( py::list l, unsigned narg )
    {
      const std::size_t n = static_cast<std::size_t>(py::len(l));
      std::vector<std::string> v;
      v.reserve(n);
      for (std::size_t i = 0; i < n; ++i)
        v.push_back( l[i].cast<std::string>() );
      return std::make_shared<GriffDataReader>(v,narg);
    }))
    .def(py::init<std::string, unsigned>())
    .def(py::init<std::vector<std::string> >())
    .def(py::init<std::vector<std::string>, unsigned>())
    .def("hasTrackID",&GriffDataReader::hasTrackID)//
    .def("goToNextFile",&GriffDataReader::goToNextFile)
    .def("goToNextEvent",&GriffDataReader::goToNextEvent)
    .def("goToFirstEvent",&GriffDataReader::goToFirstEvent)
    .def("skipEvents",&GriffDataReader::skipEvents)//
    .def("eventActive",&GriffDataReader::eventActive)
    .def("runNumber",&GriffDataReader::runNumber)
    .def("eventNumber",&GriffDataReader::eventNumber)
    .def("currentEventVersion",&GriffDataReader::currentEventVersion)
    .def("nTracks",&GriffDataReader::nTracks)
    .def("nPrimaryTracks",&GriffDataReader::nPrimaryTracks)
    .def("loopEvents",&GriffDataReader::loopEvents)
    .def("eventStorageMode",&GriffDataReader::eventStorageModeStr)
    .def("seed",&GriffDataReader::seed)
    .def("seedStr",&GriffDataReader::seedStr)
    .def("getTrack",&GriffDataReader::getTrack,py::return_value_policy::reference)
    .def("getTrackByID",&GriffDataReader::getTrackByID,py::return_value_policy::reference)
    .def("setupChanged",&GriffDataReader::setupChanged)
    .def("setup",&GriffDataReader::setup,py::return_value_policy::reference)
    .def("allowSetupChange",&GriffDataReader::allowSetupChange)
    .def("loopCount",&GriffDataReader::loopCount)
    .def_static("setOpenMsg",&GriffDataReader::setOpenMsg)
    .def_static("openMsg",&GriffDataReader::openMsg)
    .def("seekEventByIndexInCurrentFile",&GriffDataReader::seekEventByIndexInCurrentFile)
    .def("eventIndexInCurrentFile",&GriffDataReader::eventIndexInCurrentFile)
    .def("eventCheckSum",&GriffDataReader::eventCheckSum)
    .def("verifyEventDataIntegrity",&GriffDataReader::verifyEventDataIntegrity)
    ;

}
