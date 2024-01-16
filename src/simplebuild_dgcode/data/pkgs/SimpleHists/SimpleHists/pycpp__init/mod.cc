#include "Core/Python.hh"
#include "SimpleHists/HistBase.hh"
#include "SimpleHists/Hist1D.hh"
#include "SimpleHists/Hist2D.hh"
#include "SimpleHists/HistCollection.hh"
#include <pybind11/numpy.h>
#include <pybind11/operators.h>// for "py::self += float() etc.

namespace sh = SimpleHists;

namespace {
namespace SimpleHists_pycpp {

  void HistBase_dump_0args(const sh::HistBase*h) { h->dump(); }
  void HistBase_dump_1arg(const sh::HistBase*h,bool b) { h->dump(b); }
  int HistBase_histType(const sh::HistBase*h) { return h->histType(); }
  const char* HistBase_getTitle(const sh::HistBase*h) { return h->getTitle().c_str(); }
  const char* HistBase_getXLabel(const sh::HistBase*h) { return h->getXLabel().c_str(); }
  const char* HistBase_getYLabel(const sh::HistBase*h) { return h->getYLabel().c_str(); }
  const char* HistBase_getComment(const sh::HistBase*h) { return h->getComment().c_str(); }

  void Hist1D_fill_1arg(sh::Hist1D*h,double val) { h->fill(val); }
  void Hist1D_fill_2args(sh::Hist1D*h,double val,double weight) { h->fill(val,weight); }
  void Hist2D_fill_2args(sh::Hist2D*h,double valx,double valy) { h->fill(valx,valy); }
  void Hist2D_fill_3args(sh::Hist2D*h,double valx,double valy,double weight) { h->fill(valx,valy,weight); }

  int Hist1D_getPercentileBin_1arg(sh::Hist1D*h,double arg) { return h->getPercentileBin(arg); }

  py::list HistCol_getKeys(const sh::HistCollection*hc) {
    std::set<std::string> keys;
    hc->getKeys(keys);
    py::list l;
    for ( auto& e : keys )
      l.append( e );
    return l;
  }

  void HistCol_mergeCol(sh::HistCollection*hc,const sh::HistCollection*o) { hc->merge(o); }
  void HistCol_mergeStr(sh::HistCollection*hc,const std::string& s) { hc->merge(s); }
  const char* HistCol_getKey(sh::HistCollection*hc,const sh::HistBase*h) { return hc->getKey(h).c_str(); }

  using PyArrayDbl = py::array_t<double,py::array::c_style>;
  void Hist1D_fillFromBuffer_1arg(sh::Hist1D*h, PyArrayDbl py_vals) {
    h->fillMany(py_vals.data(),py_vals.size());
  }

  void Hist1D_fillFromBuffer_2args(sh::Hist1D*h,PyArrayDbl py_vals,PyArrayDbl py_weights) {
    const auto n = py_vals.size();
    if ( py_weights.size() != n ) {
      PyErr_SetString(PyExc_ValueError, "Value and weights buffers must have equal length");
      throw py::error_already_set();
    }
    h->fillMany(py_vals.data(),py_weights.data(),n);
  }
  void Hist2D_fillFromBuffer_2args(sh::Hist2D*h,PyArrayDbl py_valsx,PyArrayDbl py_valsy) {
    const auto n = py_valsx.size();
    if ( py_valsy.size() != n ) {
      PyErr_SetString(PyExc_ValueError, "X and Y value buffers must have equal length");
      throw py::error_already_set();
    }
    h->fillMany(py_valsx.data(),py_valsy.data(),n);
  }
  void Hist2D_fillFromBuffer_3args(sh::Hist2D*h,PyArrayDbl py_valsx,PyArrayDbl py_valsy, PyArrayDbl py_weights) {
    const auto n = py_valsx.size();
    if ( py_valsy.size() != n ) {
      PyErr_SetString(PyExc_ValueError, "X and Y value buffers must have equal length");
      throw py::error_already_set();
    }
    if ( py_weights.size() != n ) {
      PyErr_SetString(PyExc_ValueError, "Value and weights buffers must have equal length");
      throw py::error_already_set();
    }
    h->fillMany(py_valsx.data(),py_valsy.data(),py_weights.data(),n);
  }

  //HistBase::serialise() must return by val in py interface (less efficient than C++ interface obviously - in C++98):
  py::buffer HistBase_serialise(sh::HistBase*h)
  {
    std::string s;
    h->serialise(s);
    py::bytes o = s;
    return o;
  }

  //needed so we can construct efficient numpy arrays of the bin contents:
  py::object Hist1D_rawContentsAsBuffer(sh::Hist1D*h)
  {
    PyObject* py_buf = PyMemoryView_FromMemory((char*)h->rawContents(), h->getNBins()*sizeof(double), PyBUF_READ);
    py::object retval = py::reinterpret_borrow<py::object>(py::handle(py_buf));
    return retval;
  }
  py::object Hist1D_rawErrorsSquaredAsBuffer(sh::Hist1D*h)
  {
    PyObject* py_buf = PyMemoryView_FromMemory((char*)h->rawErrorsSquared(), h->getNBins()*sizeof(double), PyBUF_READ);
    py::object retval = py::reinterpret_borrow<py::object>(py::handle(py_buf));
    return retval;
  }
  py::object Hist2D_rawContentsAsBuffer(sh::Hist2D*h)
  {
    PyObject* py_buf = PyMemoryView_FromMemory((char*)h->rawContents(), h->getNBinsY()*h->getNBinsX()*sizeof(double), PyBUF_READ);
    py::object retval = py::reinterpret_borrow<py::object>(py::handle(py_buf));
    return retval;
  }

  sh::Hist1D* HistCol_book1Dv1(sh::HistCollection* hc,
                               unsigned nbins, double xmin, double xmax, const std::string& key)
  {
    return hc->book1D(nbins,xmin,xmax,key);
  }

  sh::Hist1D* HistCol_book1Dv2(sh::HistCollection* hc,
                               const std::string& title,
                               unsigned nbins, double xmin, double xmax, const std::string& key)
  {
    return hc->book1D(title,nbins,xmin,xmax,key);
  }

  sh::Hist2D* HistCol_book2Dv1(sh::HistCollection* hc,
                               unsigned nbinsx, double xmin, double xmax,
                               unsigned nbinsy, double ymin, double ymax, const std::string& key)
  {
    return hc->book2D(nbinsx,xmin,xmax,nbinsy,ymin,ymax,key);
  }

  sh::Hist2D* HistCol_book2Dv2(sh::HistCollection* hc,
                               const std::string& title,
                               unsigned nbinsx, double xmin, double xmax,
                               unsigned nbinsy, double ymin, double ymax, const std::string& key)
  {
    return hc->book2D(title,nbinsx,xmin,xmax,nbinsy,ymin,ymax,key);
  }

  sh::HistCounts* HistCol_bookCountsv1(sh::HistCollection* hc,const std::string& key)
  {
    return hc->bookCounts(key);
  }

  sh::HistCounts* HistCol_bookCountsv2(sh::HistCollection* hc,const std::string& title,const std::string& key)
  {
    return hc->bookCounts(title,key);
  }

  sh::HistBase* HistCol_histnonconst(sh::HistCollection* hc, const std::string& key)
  {
    return hc->hist(key);
  }
  const sh::HistBase* HistCol_histconst(const sh::HistCollection* hc, const std::string& key)
  {
    return hc->hist(key);
  }

  void HistColl_saveToFile_1arg(const sh::HistCollection* hc, const std::string& f)
  {
    hc->saveToFile(f);
  }

  sh::HistCounts::Counter HistCounts_addCounter_1arg(sh::HistCounts* hc, const std::string& l)
  {
    return hc->addCounter(l);
  }

  const char * Counter_getLabel(sh::HistCounts::Counter* c) { return c->getLabel().c_str(); }
  const char * Counter_getDisplayLabel(sh::HistCounts::Counter* c) { return c->getDisplayLabel().c_str(); }
  const char * Counter_getComment(sh::HistCounts::Counter* c) { return c->getComment().c_str(); }


  py::list HistCounts_getCounters(sh::HistCounts*h)
  {
    std::list<sh::HistCounts::Counter> l;
    h->getCounters(l);

    py::list res;

    auto itE=l.end();
    for (auto it=l.begin();it!=itE;++it) {
      res.append(*it);
    }
    return res;
  }

  sh::HistCounts::Counter HistCounts_getCounter(sh::HistCounts*h, const std::string& l)
  {
    return h->getCounter(l);
  }

}
}

namespace shp = SimpleHists_pycpp;

PYTHON_MODULE( mod )
{
  //Change the module name to avoid type(Hist1D()) giving
  //'SimpleHists._init.Hist1D' but rather give 'SimpleHists.Hist1D':
#define simplehist_str(s) #s
#define simplehist_xstr(s) simplehist_str(s)
  mod.attr("__name__") = simplehist_xstr(PACKAGE_NAME);

  py::class_<sh::HistBase> thePyHistBaseClass(mod, "HistBase");
  thePyHistBaseClass
    .def("getTitle",&shp::HistBase_getTitle)
    .def("getXLabel",&shp::HistBase_getXLabel)
    .def("getYLabel",&shp::HistBase_getYLabel)
    .def("getComment",&shp::HistBase_getComment)
    .def("setTitle",&sh::HistBase::setTitle)
    .def("setXLabel",&sh::HistBase::setXLabel)
    .def("setYLabel",&sh::HistBase::setYLabel)
    .def("setComment",&sh::HistBase::setComment)
    .def("histType",&shp::HistBase_histType)
    .def("getIntegral",&sh::HistBase::getIntegral)
    .def("empty",&sh::HistBase::empty)
    .def("dimension",&sh::HistBase::dimension)
    .def("serialise",&shp::HistBase_serialise)
    .def("dump",&sh::HistBase::dump)//version with 2 args
    .def("dump",&shp::HistBase_dump_1arg)//version with 1 arg
    .def("dump",&shp::HistBase_dump_0args)//version with 0 args
    .def("mergeCompatible",&sh::HistBase::mergeCompatible)
    .def("merge",&sh::HistBase::merge)
    .def("isSimilar",&sh::HistBase::isSimilar)
    .def("scale",&sh::HistBase::scale)
    .def("norm",&sh::HistBase::norm)
    .def("clone",&sh::HistBase::clone,py::return_value_policy::reference)
    .def("reset",&sh::HistBase::reset)
    //properties (all lowercase):
    .def_property("title", &shp::HistBase_getTitle,&sh::HistBase::setTitle)
    .def_property("xlabel", &shp::HistBase_getXLabel, &sh::HistBase::setXLabel)
    .def_property("ylabel", &shp::HistBase_getYLabel, &sh::HistBase::setYLabel)
    .def_property("comment", &shp::HistBase_getComment, &sh::HistBase::setComment)
    .def_property_readonly("integral",&sh::HistBase::getIntegral)
    ;

  mod.def("histTypeOfData",&sh::histTypeOfData);
  mod.def("deserialise",&sh::deserialise,py::return_value_policy::reference);
  mod.def("deserialiseAndManage",&sh::deserialise,py::return_value_policy::take_ownership);

  //Hist1D:
  py::class_<sh::Hist1D> thePyHist1DClass(mod, "Hist1D", thePyHistBaseClass);
  thePyHist1DClass
    .def(py::init<unsigned, double, double>(),py::arg("nbins"),py::arg("xmin"),py::arg("xmax"))
    .def(py::init<const std::string&,unsigned, double, double>(),py::arg("title"),py::arg("nbins"),py::arg("xmin"),py::arg("xmax"))
    .def(py::init([]( py::buffer arg) { return std::make_unique<sh::Hist1D>(py::cast<std::string>(arg));}), py::arg("serialised_data") )
    .def(py::pickle(
                    [](const sh::Hist1D& h) {
                      std::string s;
                      h.serialise(s);
                      py::bytes bytes(s);
                      return py::make_tuple(bytes);
                    },
                    [](py::tuple t) {
                      if (t.size() != 1)
                        throw std::runtime_error("Invalid state!");
                      std::string serialised_data = t[0].cast<std::string>();
                      if (sh::histTypeOfData(serialised_data)!=0x01)
                        throw std::runtime_error("Invalid state!");
                      return std::make_unique<sh::Hist1D>(serialised_data);
                    }))
    .def("getNBins",&sh::Hist1D::getNBins)
    .def("getBinContent",&sh::Hist1D::getBinContent)
    .def("getBinError",&sh::Hist1D::getBinError)
    .def("getBinCenter",&sh::Hist1D::getBinCenter)
    .def("getBinLower",&sh::Hist1D::getBinLower)
    .def("getBinUpper",&sh::Hist1D::getBinUpper)
    .def("getBinWidth",&sh::Hist1D::getBinWidth)
    .def("getMaxContent",&sh::Hist1D::getMaxContent)
    .def("getUnderflow",&sh::Hist1D::getUnderflow)
    .def("getOverflow",&sh::Hist1D::getOverflow)
    .def("getMinFilled",&sh::Hist1D::getMinFilled)
    .def("getMaxFilled",&sh::Hist1D::getMaxFilled)
    .def("getXMin",&sh::Hist1D::getXMin)
    .def("getXMax",&sh::Hist1D::getXMax)
    .def("getMean",&sh::Hist1D::getMean)
    .def("getRMS",&sh::Hist1D::getRMS)
    .def("getRMSSquared",&sh::Hist1D::getRMSSquared)
    .def("valueToBin",&sh::Hist1D::valueToBin)
    .def("fillN",&sh::Hist1D::fillN)
    .def("_rawfill",&shp::Hist1D_fill_1arg)
    .def("_rawfill",&shp::Hist1D_fill_2args)
    .def("_rawfillFromBuffer",&shp::Hist1D_fillFromBuffer_1arg)
    .def("_rawfillFromBuffer",&shp::Hist1D_fillFromBuffer_2args)
    .def("_rawContents",&shp::Hist1D_rawContentsAsBuffer)
    .def("_rawErrorsSquared",&shp::Hist1D_rawErrorsSquaredAsBuffer)
    .def("setErrorsByContent",&sh::Hist1D::setErrorsByContent)
    .def("rebin",&sh::Hist1D::rebin)
    .def("canRebin",&sh::Hist1D::canRebin)
    .def("resetAndRebin",&sh::Hist1D::resetAndRebin)
    .def("getPercentileBin",&sh::Hist1D::getPercentileBin)
    .def("getPercentileBin",&shp::Hist1D_getPercentileBin_1arg)
    .def("getBinSum",&sh::Hist1D::getBinSum)
    //readonly properties (all lowercase):
    .def_property_readonly("nbins",&sh::Hist1D::getNBins)
    .def_property_readonly("binwidth",&sh::Hist1D::getBinWidth)
    .def_property_readonly("underflow",&sh::Hist1D::getUnderflow)
    .def_property_readonly("overflow",&sh::Hist1D::getOverflow)
    .def_property_readonly("minfilled",&sh::Hist1D::getMinFilled)
    .def_property_readonly("maxfilled",&sh::Hist1D::getMaxFilled)
    .def_property_readonly("xmin",&sh::Hist1D::getXMin)
    .def_property_readonly("xmax",&sh::Hist1D::getXMax)
    .def_property_readonly("mean",&sh::Hist1D::getMean)
    .def_property_readonly("rms",&sh::Hist1D::getRMS)
    .def_property_readonly("rms2",&sh::Hist1D::getRMSSquared)
    .def_property_readonly("maxcontent",&sh::Hist1D::getMaxContent)
    ;

  //Hist2D:
  py::class_<sh::Hist2D> thePyHist2DClass(mod,"Hist2D",thePyHistBaseClass);
  thePyHist2DClass
    .def(py::init<unsigned, double, double, unsigned, double, double>(),
         py::arg("nbinsx"),py::arg("xmin"),py::arg("xmax"),
         py::arg("nbinsy"),py::arg("ymin"),py::arg("ymax"))
    .def(py::init<const std::string&,unsigned, double, double,unsigned, double, double>(),
         py::arg("title"),py::arg("nbinsx"),py::arg("xmin"),py::arg("xmax"),
         py::arg("nbinsy"),py::arg("ymin"),py::arg("ymax"))
    .def(py::init([]( py::buffer arg) { return std::make_unique<sh::Hist2D>(py::cast<std::string>(arg));}), py::arg("serialised_data") )
    .def(py::pickle(
                    [](const sh::Hist2D& h) {
                      std::string s;
                      h.serialise(s);
                      py::bytes bytes(s);
                      return py::make_tuple(bytes);
                    },
                    [](py::tuple t) {
                      if (t.size() != 1)
                        throw std::runtime_error("Invalid state!");
                      std::string serialised_data = t[0].cast<std::string>();
                      if (sh::histTypeOfData(serialised_data)!=0x02)
                        throw std::runtime_error("Invalid state!");
                      return std::make_unique<sh::Hist2D>(serialised_data);
                    }))
    .def("getNBinsX",&sh::Hist2D::getNBinsX)
    .def("getNBinsY",&sh::Hist2D::getNBinsY)
    .def("getBinContent",&sh::Hist2D::getBinContent)
    .def("getBinCenterX",&sh::Hist2D::getBinCenterX)
    .def("getBinLowerX",&sh::Hist2D::getBinLowerX)
    .def("getBinUpperX",&sh::Hist2D::getBinUpperX)
    .def("getBinCenterY",&sh::Hist2D::getBinCenterY)
    .def("getBinLowerY",&sh::Hist2D::getBinLowerY)
    .def("getBinUpperY",&sh::Hist2D::getBinUpperY)
    .def("getBinWidthX",&sh::Hist2D::getBinWidthX)
    .def("getBinWidthY",&sh::Hist2D::getBinWidthY)
    .def("getUnderflowX",&sh::Hist2D::getUnderflowX)
    .def("getOverflowY",&sh::Hist2D::getOverflowY)
    .def("getUnderflowX",&sh::Hist2D::getUnderflowX)
    .def("getOverflowY",&sh::Hist2D::getOverflowY)
    .def("getMinFilledX",&sh::Hist2D::getMinFilledX)
    .def("getMaxFilledX",&sh::Hist2D::getMaxFilledX)
    .def("getMinFilledY",&sh::Hist2D::getMinFilledY)
    .def("getMaxFilledY",&sh::Hist2D::getMaxFilledY)
    .def("getXMin",&sh::Hist2D::getXMin)
    .def("getXMax",&sh::Hist2D::getXMax)
    .def("getYMin",&sh::Hist2D::getYMin)
    .def("getYMax",&sh::Hist2D::getYMax)
    .def("getMeanX",&sh::Hist2D::getMeanX)
    .def("getRMSX",&sh::Hist2D::getRMSX)
    .def("getRMSSquaredX",&sh::Hist2D::getRMSSquaredX)
    .def("getMeanY",&sh::Hist2D::getMeanY)
    .def("getRMSY",&sh::Hist2D::getRMSY)
    .def("getRMSSquaredY",&sh::Hist2D::getRMSSquaredY)
    .def("getCovariance",&sh::Hist2D::getCovariance)
    .def("getCorrelation",&sh::Hist2D::getCorrelation)
    .def("valueToBinX",&sh::Hist2D::valueToBinX)
    .def("valueToBinY",&sh::Hist2D::valueToBinY)
    .def("_rawfill",&shp::Hist2D_fill_2args)
    .def("_rawfill",&shp::Hist2D_fill_3args)
    .def("_rawfillFromBuffer",&shp::Hist2D_fillFromBuffer_2args)
    .def("_rawfillFromBuffer",&shp::Hist2D_fillFromBuffer_3args)
    .def("_rawContents",&shp::Hist2D_rawContentsAsBuffer)
    //readonly properties (all lowercase):
    .def_property_readonly("binwidthx",&sh::Hist2D::getBinWidthX)
    .def_property_readonly("binwidthy",&sh::Hist2D::getBinWidthY)
    .def_property_readonly("nbinsx",&sh::Hist2D::getNBinsX)
    .def_property_readonly("nbinsy",&sh::Hist2D::getNBinsY)
    .def_property_readonly("underflowx",&sh::Hist2D::getUnderflowX)
    .def_property_readonly("overflowx",&sh::Hist2D::getOverflowX)
    .def_property_readonly("underflowy",&sh::Hist2D::getUnderflowY)
    .def_property_readonly("overflowy",&sh::Hist2D::getOverflowY)
    .def_property_readonly("minfilledx",&sh::Hist2D::getMinFilledX)
    .def_property_readonly("maxfilledx",&sh::Hist2D::getMaxFilledX)
    .def_property_readonly("minfilledy",&sh::Hist2D::getMinFilledY)
    .def_property_readonly("maxfilledy",&sh::Hist2D::getMaxFilledY)
    .def_property_readonly("xmin",&sh::Hist2D::getXMin)
    .def_property_readonly("xmax",&sh::Hist2D::getXMax)
    .def_property_readonly("ymin",&sh::Hist2D::getYMin)
    .def_property_readonly("ymax",&sh::Hist2D::getYMax)
    .def_property_readonly("meanx",&sh::Hist2D::getMeanX)
    .def_property_readonly("rmsx",&sh::Hist2D::getRMSX)
    .def_property_readonly("rms2x",&sh::Hist2D::getRMSSquaredX)
    .def_property_readonly("meany",&sh::Hist2D::getMeanY)
    .def_property_readonly("rmsy",&sh::Hist2D::getRMSY)
    .def_property_readonly("rms2y",&sh::Hist2D::getRMSSquaredY)
    .def_property_readonly("covariance",&sh::Hist2D::getCovariance)
    .def_property_readonly("covxy",&sh::Hist2D::getCovariance)
    .def_property_readonly("correlation",&sh::Hist2D::getCorrelation)
    .def_property_readonly("corxy",&sh::Hist2D::getCorrelation)
    ;

  //HistCounts::Counter:
  py::class_<sh::HistCounts::Counter>(mod, "Counter")
    .def("getValue",&sh::HistCounts::Counter::getValue)
    .def("__call__",&sh::HistCounts::Counter::getValue)
    .def("getError",&sh::HistCounts::Counter::getError)
    .def("getErrorSquared",&sh::HistCounts::Counter::getErrorSquared)
    .def("getLabel",&shp::Counter_getLabel)
    .def("getDisplayLabel",&shp::Counter_getDisplayLabel)
    .def("setDisplayLabel",&sh::HistCounts::Counter::setDisplayLabel)
    .def("getComment",&shp::Counter_getComment)
    .def("setComment",&sh::HistCounts::Counter::setComment)
    .def(py::self += float())
    .def(py::self += py::self)
    //readonly properties (all lowercase):
    .def_property_readonly("value",&sh::HistCounts::Counter::getValue)
    .def_property_readonly("error",&sh::HistCounts::Counter::getError)
    .def_property_readonly("errorsquared",&sh::HistCounts::Counter::getErrorSquared)
    .def_property_readonly("label",&shp::Counter_getLabel)
    .def_property("displaylabel",&shp::Counter_getDisplayLabel,&sh::HistCounts::Counter::setDisplayLabel)
    .def_property("comment",&shp::Counter_getComment,&sh::HistCounts::Counter::setComment)
    ;

  //HistCounts:
  py::class_<sh::HistCounts> thePyHistCountsClass(mod, "HistCounts",thePyHistBaseClass);
  thePyHistCountsClass
    .def(py::init<>())
    .def(py::init([]( py::str arg) { auto h = std::make_unique<sh::HistCounts>(); h->setTitle( py::cast<std::string>(arg) ); return h; }), py::arg("title") )
    .def(py::init([]( py::buffer arg) { return std::make_unique<sh::HistCounts>(py::cast<std::string>(arg));}), py::arg("serialised_data") )
    .def(py::pickle(
                    [](const sh::HistCounts& h) {
                      std::string s;
                      h.serialise(s);
                      py::bytes bytes(s);
                      return py::make_tuple(bytes);
                    },
                    [](py::tuple t) {
                      if (t.size() != 1)
                        throw std::runtime_error("Invalid state!");
                      std::string serialised_data = t[0].cast<std::string>();
                      if (sh::histTypeOfData(serialised_data)!=0x03)
                        throw std::runtime_error("Invalid state!");
                      return std::make_unique<sh::HistCounts>(serialised_data);
                    }))
    .def("addCounter",&sh::HistCounts::addCounter)
    .def("addCounter",&shp::HistCounts_addCounter_1arg)
    .def("getMaxContent",&sh::HistCounts::getMaxContent)
    .def("setErrorsByContent",&sh::HistCounts::setErrorsByContent)
    .def("sortByLabels",&sh::HistCounts::sortByLabels)
    .def("sortByDisplayLabels",&sh::HistCounts::sortByDisplayLabels)
    .def("getCounters",&shp::HistCounts_getCounters)
    .def("getCounter",&shp::HistCounts_getCounter)
    .def("hasCounter",&sh::HistCounts::hasCounter)
    .def("nCounters",&sh::HistCounts::nCounters)
    //readonly properties (all lowercase):
    .def_property_readonly("ncounters",&sh::HistCounts::nCounters)
    .def_property_readonly("maxcontent",&sh::HistCounts::getMaxContent)
    .def_property_readonly("counters",&shp::HistCounts_getCounters)
    ;

  //HistCollection:
  py::class_<sh::HistCollection>(mod,"HistCollection")
    .def(py::init<>())
    .def(py::init<const std::string&>(),py::arg("filename"))
    .def("book1D",&shp::HistCol_book1Dv1,py::return_value_policy::reference)
    .def("book1D",&shp::HistCol_book1Dv2,py::return_value_policy::reference)
    .def("book2D",&shp::HistCol_book2Dv1,py::return_value_policy::reference)
    .def("book2D",&shp::HistCol_book2Dv2,py::return_value_policy::reference)
    .def("bookCounts",&shp::HistCol_bookCountsv1,py::return_value_policy::reference)
    .def("bookCounts",&shp::HistCol_bookCountsv2,py::return_value_policy::reference)
    .def("hasKey",&sh::HistCollection::hasKey)
    .def("getKey",&shp::HistCol_getKey)
    .def("hist",&shp::HistCol_histnonconst,py::return_value_policy::reference)
    .def("hist",&shp::HistCol_histconst,py::return_value_policy::reference)
    .def("add",&sh::HistCollection::add)
    .def("remove",&sh::HistCollection::remove,py::return_value_policy::reference)
    .def("removeAndManage",&sh::HistCollection::remove,py::return_value_policy::take_ownership)
    .def("saveToFile",&sh::HistCollection::saveToFile)
    .def("saveToFile",&shp::HistColl_saveToFile_1arg)
    .def("getKeys",&shp::HistCol_getKeys)
    .def("merge",&shp::HistCol_mergeCol)
    .def("merge",&shp::HistCol_mergeStr)
    .def("isSimilar",&sh::HistCollection::isSimilar)
    .def_property_readonly("keys",&shp::HistCol_getKeys)
    ;

}

