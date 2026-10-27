#include "sbldpy/sbldpy.hh"
#include "G4Interfaces/CppObjectHandles.hh"
#include "Mesh/Mesh.hh"
#include "G4ExprParser/G4SteppingASTBuilder.hh"
#include "G4Interfaces/FrameworkGlobals.hh"

#include "G4Utils/GeoUtils.hh"
#include "G4UserSteppingAction.hh"
#include "G4UserEventAction.hh"
#include "G4Step.hh"

#include <sys/types.h>
#include <unistd.h>
#include <memory>
#include <tuple>

// HeatMapWriter is the user visible class which will be instantiated by the user
// on the python side (see G4HeatMap/__init__.py) and has its "inithook" method
// hooked into the framework by the user. Several of these can exist in case the user desires several meshes written.
//
// A single HeatMapSteppingAction is used to hook into Geant4 and distribute
// G4Steps to all of these writers.

namespace DMWriter {

  class HeatMapWriter;

  using HeatMapWriterPtr = std::shared_ptr<HeatMapWriter>;

  class HeatMapSteppingAction : public G4UserSteppingAction
  {
  public:
    //Returns handles of new user actions (see G4Interfaces/CppObjectHandles.hh)
    //to be installed, or zeros if they were already created:
    using ActionHandles = std::tuple<std::int64_t,std::int64_t>;
    static ActionHandles registerWriter( HeatMapWriterPtr );
    virtual void UserSteppingAction(const G4Step* step);
    static void beginEvt();
    static double stat_nevts() { assert(m_theInstance); return m_theInstance->m_meshstat_nevts; }
    static double stat_nsteps() { assert(m_theInstance); return m_theInstance->m_meshstat_nsteps; }
    static double stat_wsteps() { assert(m_theInstance); return m_theInstance->m_meshstat_wsteps; }
  private:
    HeatMapSteppingAction()
      : m_meshstat_nevts(0), m_meshstat_nsteps(0), m_meshstat_wsteps(0) {}
    virtual ~HeatMapSteppingAction();
    std::vector<HeatMapWriterPtr> m_heatmapwriters;
    static HeatMapSteppingAction * m_theInstance;
    double m_meshstat_nevts;
    double m_meshstat_nsteps;
    double m_meshstat_wsteps;
  };

  HeatMapSteppingAction * HeatMapSteppingAction::m_theInstance = 0;

  struct HeatMapEventAction : public G4UserEventAction
  {
    HeatMapEventAction() : G4UserEventAction() {}
    virtual ~HeatMapEventAction() {}
    virtual void BeginOfEventAction(const G4Event*)
    {
      HeatMapSteppingAction::beginEvt();
    }
  };

  class HeatMapWriter : public std::enable_shared_from_this<HeatMapWriter> {
    //Class exposed to python for users to enable and configure heatmap writing.
  public:
    HeatMapWriter( const char * filename,
                   long nx, double xlow, double xup,
                   long ny, double ylow, double yup,
                   long nz, double zlow, double zup );

    //Will create limits (xlow, xup, ylow, yup, zlow, zup) based on global
    //extent of geometry:
    HeatMapWriter( const char * filename, long nx, long ny, long nz );
    void delayedInitIfNeeded();

    virtual void processG4Step(const G4Step* step, double weight);
    virtual ~HeatMapWriter() = default;
    //Returns handles of user actions to install (see registerWriter):
    HeatMapSteppingAction::ActionHandles inithook();
    void setComments(const char *);
    void setFilterExpression(const char *);
    void setQuantityExpression(const char *);
    void ensureWrite();
    void merge();
    std::string cacheFile(unsigned iproc) const;
    const std::string& outputFile() const { return m_outputFile; }

  private:
    std::string constructName() const;
    void commonInit(const char * filename);
    void meshInit(const long(&n)[3], const double(&lw)[3], const double(&up)[3]);
    Mesh::Mesh<3> m_mesh;
    long m_delayInitNCells[3];
    std::string m_outputFile;
    std::string m_tmpFileBase;
    bool m_wrotefile;
    std::string m_comments;

    G4ExprParser::G4SteppingASTBuilder m_expr_builder;
    ExprParser::Evaluator<bool> m_eval_filter;
    ExprParser::Evaluator<ExprParser::float_type> m_eval_quantity;
    std::string m_expr_filter;
    std::string m_expr_quantity;
  };

  HeatMapSteppingAction::~HeatMapSteppingAction()
  {
    for ( auto& e : m_theInstance->m_heatmapwriters )
      e->ensureWrite();
    if (!FrameworkGlobals::isForked()||FrameworkGlobals::isParent()) {
      printf("HeatMapWriter: ==> All requested heatmap files were created successfully.\n");
      printf("HeatMapWriter: ==> Browse the files immediately with the sb_mesh3d_browse command.\n");
      printf("HeatMapWriter: ==> More info is at https://mctools.github.io/simplebuild-dgcode/heatmap.html\n");
    }
  }

  void HeatMapSteppingAction::beginEvt() {
    assert (m_theInstance);
    ++(m_theInstance->m_meshstat_nevts);
  }

  HeatMapSteppingAction::ActionHandles HeatMapSteppingAction::registerWriter( HeatMapWriterPtr writer )
  {
    ActionHandles res{ 0, 0 };
    if (!m_theInstance) {
      m_theInstance = new HeatMapSteppingAction;
      G4UserSteppingAction* stepact = m_theInstance;
      G4UserEventAction * evtact = new HeatMapEventAction;
      res = ActionHandles{ G4Interfaces::registerObject( stepact ), G4Interfaces::registerObject( evtact ) };
    }
    for ( auto& e : m_theInstance->m_heatmapwriters ) {
      if ( e == writer )
        throw std::runtime_error("Attempting to register the same HeatMapWriter more than once");
      if ( e->outputFile() == writer->outputFile() )
        throw std::runtime_error("Attempting to register two HeatMapWriters writing to the same file");
    }
    m_theInstance->m_heatmapwriters.push_back( writer );
    return res;
  }

  void HeatMapSteppingAction::UserSteppingAction(const G4Step* step)
  {
    double w = step->GetPostStepPoint()->GetWeight();//NB: this rather than ->GetTrack()->GetWeight()
    ++m_meshstat_nsteps;
    m_meshstat_wsteps += w;
    if (w) {
      for ( auto& e : m_theInstance->m_heatmapwriters )
        e->processG4Step(step,w);
    }
  }

  void HeatMapWriter::meshInit(const long(&n)[3], const double(&lw)[3], const double(&up)[3])
  {
    std::string name = constructName();
    m_mesh.reinit(n,lw,up,name.c_str(),m_comments.c_str());
    m_mesh.setCellUnits("mm");
  }

  void HeatMapWriter::commonInit(const char * filename)
  {
    m_outputFile = filename;
    m_wrotefile = false;
    if (m_outputFile.size()<7||strcmp(&m_outputFile.at(m_outputFile.size()-7),".mesh3d")!=0)
      m_outputFile += ".mesh3d";
    std::stringstream tmp;
    tmp << m_outputFile << ".tmpcache_" << (std::uint64_t)(getpid())<<"_proc";
    m_tmpFileBase = tmp.str();
    setFilterExpression("true");
    setQuantityExpression("step.edep");
  }

  HeatMapWriter::HeatMapWriter( const char * filename,
                                long nx, double xlow, double xup,
                                long ny, double ylow, double yup,
                                long nz, double zlow, double zup )
  {
    commonInit(filename);
    long ncells[3];
    double cell_lower[3];
    double cell_upper[3];
    ncells[0] = nx;
    ncells[1] = ny;
    ncells[2] = nz;
    cell_lower[0] = xlow;
    cell_lower[1] = ylow;
    cell_lower[2] = zlow;
    cell_upper[0] = xup;
    cell_upper[1] = yup;
    cell_upper[2] = zup;
    meshInit(ncells,cell_lower,cell_upper);
    m_delayInitNCells[0] = m_delayInitNCells[1] = m_delayInitNCells[2] = 0;
  }

  HeatMapWriter::HeatMapWriter( const char * filename,
                                long nx,
                                long ny,
                                long nz )
  {
    commonInit(filename);
    m_delayInitNCells[0] = nx;
    m_delayInitNCells[1] = ny;
    m_delayInitNCells[2] = nz;
  }

  void HeatMapWriter::delayedInitIfNeeded()
  {
    if (m_delayInitNCells[0]==0)
      return;
    double cell_lower[3];
    double cell_upper[3];
    G4Utils::autodetect_world_extent(cell_lower,cell_upper);
    printf("HeatMapWriter: Autodetected world extent along x-axis : %g to %g\n",cell_lower[0],cell_upper[0]);
    printf("HeatMapWriter: Autodetected world extent along y-axis : %g to %g\n",cell_lower[1],cell_upper[1]);
    printf("HeatMapWriter: Autodetected world extent along z-axis : %g to %g\n",cell_lower[2],cell_upper[2]);
    meshInit(m_delayInitNCells,cell_lower,cell_upper);
    m_delayInitNCells[0] = m_delayInitNCells[1] = m_delayInitNCells[2] = 0;
  }

  void HeatMapWriter::processG4Step(const G4Step* step, double weight) {
    //TODO: We could achieve less levels of indirection by having the stepping
    //action own m_expr_builder and copy m_eval_filter onto our stepping action
    //object as well (in an array with large max size, not a vector).
    //
    //Furthermore we could recognise constant factors in m_eval_quantity and
    //only apply them once per cell at the end (for cheaper custom units).
    m_expr_builder.setCurrentStep(step);
    if (!m_eval_filter())
      return;
    double val = m_eval_quantity();
    val *= weight;
    if (!val)
      return;
    const G4ThreeVector& pos0 = step->GetPreStepPoint()->GetPosition();
    const G4ThreeVector& pos1 = step->GetPostStepPoint()->GetPosition();
    assert(sizeof(pos0)==3*sizeof(double));
    m_mesh.filler().fill( val,
                          reinterpret_cast<const double(&)[3]>(pos0),
                          reinterpret_cast<const double(&)[3]>(pos1) );
  }

  HeatMapSteppingAction::ActionHandles HeatMapWriter::inithook() {

    delayedInitIfNeeded();

    //register to get g4 stepping callbacks and a ensure_write_file call after G4
    //loop (the Python side installs the actions and registers a merge callback,
    //see G4HeatMap/__init__.py):
    return HeatMapSteppingAction::registerWriter(this->shared_from_this());
  }

  void HeatMapWriter::setComments(const char * c)
  {
    m_comments = c;
    m_mesh.setComments(c);
  }

  std::string HeatMapWriter::constructName() const
  {
    std::string name = m_expr_quantity;
    if (m_eval_filter.isConstant())
      name += m_eval_filter() ? " [all steps]" : " [no steps]";
    else
      name += " [where "+m_expr_filter+ "]";
    return name;
  }

  void HeatMapWriter::setFilterExpression(const char * expr)
  {
    m_expr_filter = expr;
    try {
      m_eval_filter = m_expr_builder.createEvaluator<bool>(m_expr_filter);
    } catch (ExprParser::InputError& e) {
      printf("\nHeatMapWriter ERROR: Invalid filter expression \"%s\"\n",m_expr_filter.c_str());
      printf("HeatMapWriter ERROR: %s : %s\n\n",e.epType(),e.epWhat());
      throw;
    }
    if (m_eval_filter.isConstant()&&!m_eval_filter())
      printf("HeatMapWriter: WARNING - Specified filter \"%s\" always evaluates to false\n",expr);
    std::string name = constructName();
    m_mesh.setName(name.c_str());
  }

  void HeatMapWriter::setQuantityExpression(const char * expr)
  {
    m_expr_quantity = expr;
    try {
      m_eval_quantity = m_expr_builder.createEvaluator<ExprParser::float_type>(m_expr_quantity);
    } catch (ExprParser::InputError& e) {
      printf("\nHeatMapWriter ERROR: Invalid quantity expression \"%s\"\n",m_expr_filter.c_str());
      printf("HeatMapWriter ERROR: %s : %s\n\n",e.epType(),e.epWhat());
      throw;
    }
    if (m_eval_quantity.isConstant())
      printf("HeatMapWriter: WARNING - Specified quantity \"%s\" evaluates to a constant value (%g)\n",expr,m_eval_quantity());
    std::string name = constructName();
    m_mesh.setName(name.c_str());
  }

  void HeatMapWriter::ensureWrite()
  {
    if (m_wrotefile)
      return;
    m_wrotefile = true;

    //Set collected statistics:
    m_mesh.enableStat("nevts") = HeatMapSteppingAction::stat_nevts();
    m_mesh.enableStat("nsteps") = HeatMapSteppingAction::stat_nsteps();
    m_mesh.enableStat("wsteps") = HeatMapSteppingAction::stat_wsteps();

    std::string fn = m_outputFile;
    if (FrameworkGlobals::isForked()) {
      if (FrameworkGlobals::isParent())
        return;//parent in MP job only writes final file after merging.
      fn = cacheFile(FrameworkGlobals::mpID());
      printf("HeatMapWriter: Writing intermediate result from proc%i\n",FrameworkGlobals::mpID());
    } else {
      printf("HeatMapWriter: Writing result to %s\n",fn.c_str());
    }
    m_mesh.saveToFile(fn);
    m_mesh.filler().data().clear();
    if (!FrameworkGlobals::isForked())
      printf("HeatMapWriter: Done\n");
  }

  void HeatMapWriter::merge() {
    ensureWrite();

    //Ok, we now have nprocs-1 temporary output files from child procs from which
    //we must merge results (always in the same order due to non-commutativity of
    //floating point addition).

    unsigned nprocs = FrameworkGlobals::nProcs();
    for (unsigned i = 1; i < nprocs; ++i) {
      std::string fnother = cacheFile(i);
      printf("HeatMapWriter: Merging output of proc%i into proc0\n",i);
      m_mesh.merge(fnother);
      int rr = remove(fnother.c_str());
      if (rr)
        printf("HeatMapWriter: WARNING - Could not remove file after merging: %s\n",fnother.c_str());

    }
    printf("HeatMapWriter: Done merging output from %i processes.\n",nprocs);
    printf("HeatMapWriter: Writing result to %s\n",m_outputFile.c_str());
    m_mesh.saveToFile(m_outputFile);
    m_mesh.filler().data().clear();
    printf("HeatMapWriter: Done\n");
  }

  std::string HeatMapWriter::cacheFile(unsigned iproc) const
  {
    std::stringstream tmp;
    tmp << m_tmpFileBase << iproc;
    return tmp.str();
  }
}

namespace DMWriter {

  //Copyable holder class (HeatMapWriter must be owned by a shared_ptr):
  class PyHeatMapWriter {
  public:
    PyHeatMapWriter( std::shared_ptr<HeatMapWriter> w ) : m_w( std::move( w ) ) {}
    PyHeatMapWriter( const std::string& filename,
                     std::int64_t nx, double xlow, double xup,
                     std::int64_t ny, double ylow, double yup,
                     std::int64_t nz, double zlow, double zup )
      : m_w( std::make_shared<HeatMapWriter>( filename.c_str(), nx, xlow, xup, ny, ylow, yup, nz, zlow, zup ) )
    {
    }
    HeatMapSteppingAction::ActionHandles inithook() { return m_w->inithook(); }
    void merge() { m_w->merge(); }
    void setQuantity( const std::string& e ) { m_w->setQuantityExpression( e.c_str() ); }
    void setFilter( const std::string& e ) { m_w->setFilterExpression( e.c_str() ); }
    void setComments( const std::string& c ) { m_w->setComments( c.c_str() ); }
  private:
    std::shared_ptr<HeatMapWriter> m_w;
  };

}

SBLDPY_MODULE( _impl )
{
  using DMWriter::PyHeatMapWriter;
  m.add_class<PyHeatMapWriter>("HeatMapWriter")
    .def_init<const std::string&,std::int64_t,double,double,std::int64_t,double,double,std::int64_t,double,double>(
      { "filename", "nx", "xlow", "xup", "ny", "ylow", "yup", "nz", "zlow", "zup" } )
    .def("inithook",&PyHeatMapWriter::inithook,
         "Register writer, returning handles of user actions to install (zeros if already installed)")
    .def("merge",&PyHeatMapWriter::merge)
    .def("setQuantity",&PyHeatMapWriter::setQuantity,{ "expression" })
    .def("setFilter",&PyHeatMapWriter::setFilter,{ "expression" })
    .def("setComments",&PyHeatMapWriter::setComments,{ "comments" });
  m.def("create_auto",[]( const std::string& filename, std::int64_t nx, std::int64_t ny, std::int64_t nz )
  {
    return PyHeatMapWriter( std::make_shared<DMWriter::HeatMapWriter>( filename.c_str(), nx, ny, nz ) );
  },"Create HeatMapWriter with limits based on the extent of the geometry",{ "filename", "nx", "ny", "nz" } );
}
