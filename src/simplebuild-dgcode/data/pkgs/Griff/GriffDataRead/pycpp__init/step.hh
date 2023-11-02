namespace GDR_step {
  namespace {
    using Step = GriffDataRead::Step;

    //Due to lack of overloading, we add dump methods directly to the objects in python:
    void dump_step(Step*s) { dump(s); }

    py::tuple step_preMomentum(Step*s) { return py::make_tuple(s->preMomentumX(),s->preMomentumY(),s->preMomentumZ()); }
    py::tuple step_preLocal(Step*s) { return py::make_tuple(s->preLocalX(),s->preLocalY(),s->preLocalZ()); }
    py::tuple step_preGlobal(Step*s) { return py::make_tuple(s->preGlobalX(),s->preGlobalY(),s->preGlobalZ()); }
    py::tuple step_postMomentum(Step*s) { return py::make_tuple(s->postMomentumX(),s->postMomentumY(),s->postMomentumZ()); }
    py::tuple step_postLocal(Step*s) { return py::make_tuple(s->postLocalX(),s->postLocalY(),s->postLocalZ()); }
    py::tuple step_postGlobal(Step*s) { return py::make_tuple(s->postGlobalX(),s->postGlobalY(),s->postGlobalZ()); }

    void pyexport( pybind11::module_ themod )
    {
      py::class_<Step,std::unique_ptr<Step, BlankDeleter<Step>> >(themod,"Step")
        .def("getTrack",&Step::getTrack,py::return_value_policy::reference)
        .def("getSegment",&Step::getSegment,py::return_value_policy::reference)
        .def("iStep",&Step::iStep)
        .def("getNextStep",&Step::getNextStep,py::return_value_policy::reference)
        .def("getPreviousStep",&Step::getPreviousStep,py::return_value_policy::reference)
        .def("eDep",&Step::eDep)
        .def("eDepNonIonising",&Step::eDepNonIonising)
        .def("preGlobalX",&Step::preGlobalX)
        .def("preGlobalY",&Step::preGlobalY)
        .def("preGlobalZ",&Step::preGlobalZ)
        .def("postGlobalX",&Step::postGlobalX)
        .def("postGlobalY",&Step::postGlobalY)
        .def("postGlobalZ",&Step::postGlobalZ)
        .def("preLocalX",&Step::preLocalX)
        .def("preLocalY",&Step::preLocalY)
        .def("preLocalZ",&Step::preLocalZ)
        .def("postLocalX",&Step::postLocalX)
        .def("postLocalY",&Step::postLocalY)
        .def("postLocalZ",&Step::postLocalZ)
        .def("preMomentumX",&Step::preMomentumX)
        .def("preMomentumY",&Step::preMomentumY)
        .def("preMomentumZ",&Step::preMomentumZ)
        .def("postMomentumX",&Step::postMomentumX)
        .def("postMomentumY",&Step::postMomentumY)
        .def("postMomentumZ",&Step::postMomentumZ)
        .def("preTime",&Step::preTime)
        .def("postTime",&Step::postTime)
        .def("preEKin",&Step::preEKin)
        .def("postEKin",&Step::postEKin)
        .def("preAtVolEdge",&Step::preAtVolEdge)
        .def("postAtVolEdge",&Step::postAtVolEdge)
        .def("preProcessDefinedStep",&Step::preProcessDefinedStepCStr)
        .def("postProcessDefinedStep",&Step::postProcessDefinedStepCStr)
        .def("stepLength",&Step::stepLength)
        .def("dump",&dump_step)
        .def("preMomentum",&step_preMomentum)
        .def("preLocal",&step_preLocal)
        .def("preGlobal",&step_preGlobal)
        .def("postMomentum",&step_postMomentum)
        .def("postLocal",&step_postLocal)
        .def("postGlobal",&step_postGlobal)
        ;

    }
  }
}
