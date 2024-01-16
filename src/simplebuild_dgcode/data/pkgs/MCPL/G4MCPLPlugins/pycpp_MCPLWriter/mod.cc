#include "Core/Python.hh"

#include "G4MCPL/G4MCPLUserFlags.hh"
#include "G4Interfaces/FrameworkGlobals.hh"
#include "G4Interfaces/GeoConstructBase.hh"
#include "G4Interfaces/ParticleGenBase.hh"
#include "G4ExprParser/G4SteppingASTBuilder.hh"
#include "MCPL/mcpl.h"

#include "G4VSensitiveDetector.hh"
#include "G4Step.hh"
#include "G4SDManager.hh"
#include "G4RegionStore.hh"

#include <set>
#include <stdexcept>
#include <string.h>
#include <sstream>

namespace G4MCPLWriter {

  namespace EP=ExprParser;

  class MCPLSensitiveDetector;
  typedef std::shared_ptr<G4ExprParser::G4SteppingASTBuilder> BuilderPtr;

  class MCPLOutputMerger {
  public:
    MCPLOutputMerger(MCPLSensitiveDetector* sd) : m_sd(sd) {}
    ~MCPLOutputMerger(){}
    void merge();
  private:
    MCPLSensitiveDetector * m_sd;
  };

  enum KillStrategy { KS_NEVER, KS_ALWAYS, KS_FILTERED };
  KillStrategy ks_from_string(const std::string& s)
  {
    if (s=="NEVER") return KS_NEVER;
    if (s=="ALWAYS") return KS_ALWAYS;
    if (s=="FILTERED") return KS_FILTERED;
    throw std::runtime_error("Invalid kill strategy. Valid options are \"NEVER\", \"ALWAYS\" or \"FILTERED\"");
  }

  // not used:
  // std::string ks_to_string(KillStrategy ks)
  // {
  //   switch (ks) {
  //   case KS_NEVER: return "NEVER";
  //   case KS_ALWAYS: return "ALWAYS";
  //   case KS_FILTERED: return "FILTERED";
  //   default:
  //     throw std::runtime_error("implementation error");
  //   }
  // }

  class MCPLSensitiveDetector: public G4VSensitiveDetector
  {
    BuilderPtr m_expr_builder;
    EP::Evaluator<bool> m_eval_filter;
    EP::Evaluator<EP::int_type> m_eval_flags;
    MCPLSensitiveDetector * m_child_sd;//support multiple MCPLWriters monitoring the same volume
  public:
    MCPLSensitiveDetector (const std::string& name,
                           const std::string& thefilename,
                           bool writedoubleprec,
                           bool writepolarisation,
                           bool writeuserflags,
                           double universalweight,
                           bool writeonvolexit,
                           KillStrategy killstrategy,
                           BuilderPtr builder,
                           EP::Evaluator<bool> filter_evaluator,
                           EP::Evaluator<EP::int_type> flag_evaluator,
                           std::vector<std::pair<std::string,std::string>> comments_and_blobs)
      : G4VSensitiveDetector(name),
        m_expr_builder(builder),
        m_eval_filter(filter_evaluator),
        m_eval_flags(flag_evaluator),
        m_child_sd(0),
        m_filename(thefilename),
        m_opt_writedoubleprec(writedoubleprec),
        m_opt_writepolarisation(writepolarisation),
        m_opt_writeuserflags(writeuserflags),
        m_opt_writeonvolexit(writeonvolexit),
        m_opt_killstrategy(killstrategy),
        m_opt_universalweight(universalweight),
        m_initialised(false),
        m_closed(false),
        m_om(0),
        m_comments_and_blobs(comments_and_blobs)
    {
      std::memset(&m_p,0,sizeof(m_p));
    }

    const std::string& filename() const { return m_filename; }

    void addChild(MCPLSensitiveDetector*child) {
      if (m_child_sd==child)
        return;
      if (m_child_sd)
        m_child_sd->addChild(child);
      else
        m_child_sd = child;
    }

    void abort()
    {
      //used by mcplwriter if something failed during init to avoid ploughing
      //ahead and creating invalid output.
      assert(!m_initialised&&!m_closed);
      m_initialised = m_closed = true;
    }
    void ensure_close_file()
    {
      if (!m_initialised)
        initmcpl();
      assert(m_initialised);
      if (m_closed)
        return;
      m_closed = true;
      assert(m_f.internal);
      if (!FrameworkGlobals::isForked())
        mcpl_closeandgzip_outfile(m_f);
      else
        mcpl_close_outfile(m_f);
      m_f.internal = 0;
    }

    virtual ~MCPLSensitiveDetector()
    {
      ensure_close_file();
      delete m_om;
    }

    virtual G4bool ProcessHits(G4Step * step,G4TouchableHistory*)
    {
      if (m_child_sd)
        m_child_sd->ProcessHits(step,0);

      if (m_closed)//in case of early abort
        return false;

      if (!m_initialised)
        initmcpl();

      G4Track * trk = step->GetTrack();
      G4StepPoint * pt = m_opt_writeonvolexit ? step->GetPostStepPoint() : step->GetPreStepPoint();
      bool atvoledge = (pt->GetStepStatus() == fGeomBoundary || pt->GetStepStatus() == fWorldBoundary);

      bool accept(true);

      if (!atvoledge) {
        //Not at volume edge. Only steps we consider here are the very first
        //step of primary particles, to make it possible to get a copy of source
        //particles:
        if ( ! ( !m_opt_writeonvolexit && trk->GetCurrentStepNumber() == 1 && !trk->GetParentID() ) ) {
          accept = false;
        }
      }


      if ( !m_opt_writeonvolexit && trk->GetParentID() && trk->GetCurrentStepNumber() == 1 ) {
        //Avoid double-counting by never storing
        //secondaries created when entering a volume:
        accept = false;
      }

      if (accept) {
        //Check filter:
        m_expr_builder->setCurrentStep(step);
        accept = m_eval_filter();
      }

      if (!accept) {
        if (m_opt_killstrategy==KS_ALWAYS) {
          trk->SetTrackStatus(fStopAndKill);
          return true;
        } else {
          return false;
        }
      }

      //Store step in MCPL file.

      auto dynpar = trk->GetDynamicParticle();
      m_p.pdgcode = dynpar->GetPDGcode();
      if (!m_p.pdgcode&&dynpar->GetParticleDefinition()->GetParticleName()=="opticalphoton") {
        m_p.pdgcode=22;//store optical photons as regular photons
      }

      m_p.time = pt->GetGlobalTime() / CLHEP::millisecond;
      m_p.weight = pt->GetWeight();
      assert(CLHEP::MeV==1.0);
      m_p.ekin = pt->GetKineticEnergy();//already in MeV
      const G4ThreeVector& pos = pt->GetPosition();
      const G4ThreeVector& dir = pt->GetMomentumDirection();
      const double tocm(1.0/CLHEP::cm);
      m_p.position[0] = pos.x() * tocm;
      m_p.position[1] = pos.y() * tocm;
      m_p.position[2] = pos.z() * tocm;
      m_p.direction[0] = dir.x();
      m_p.direction[1] = dir.y();
      m_p.direction[2] = dir.z();
      double dm2 = dir.mag2();
      if (fabs(dm2-1.0)>1.0e-12) {
        if (!dm2) {
          if (m_p.ekin) {
            //inconsistent
            printf("MCPLWriter: Warning - ekin>0 but momentumdirection=0. Will set momentumdirection to (0,0,1)\n");
          }
          //arguably consistent if ekin=0, but we should in any case only put unit
          //directional vectors in mcpl:
          m_p.direction[0] = m_p.direction[1] = 0.0;
          m_p.direction[2] = 1.0;
        } else {
          //fix normalisation:
          dm2 = 1.0/sqrt(dm2);
          m_p.direction[0] *= dm2;
          m_p.direction[1] *= dm2;
          m_p.direction[2] *= dm2;
        }
      }

      if (m_opt_writepolarisation) {
        //units??
        const G4ThreeVector& pol = pt->GetPolarization();
        m_p.polarisation[0] = pol.x();
        m_p.polarisation[1] = pol.y();
        m_p.polarisation[2] = pol.z();
      }
      if (m_opt_writeuserflags) {
        if (m_eval_flags.arg())
          m_p.userflags = (int32_t)m_eval_flags();
        else
          m_p.userflags = G4MCPLUserFlags::getFlags(trk);
      }
      mcpl_add_particle(m_f,&m_p);

      //Kill stored steps, unless asked to never kill:
      if (m_opt_killstrategy!=KS_NEVER)
        trk->SetTrackStatus(fStopAndKill);

      return true;
    }

  private:
    void initmcpl() {
      assert(!m_initialised);
      m_initialised = true;
      std::string fn = m_filename;
      if (FrameworkGlobals::isForked()&&!FrameworkGlobals::isParent()) {
        std::stringstream tmp;
        tmp << fn << "." << FrameworkGlobals::mpID();
        fn = tmp.str();
      }
      m_f = mcpl_create_outfile(fn.c_str());
      mcpl_hdr_set_srcname(m_f,"Geant4");
      for (auto it=m_comments_and_blobs.begin();it!=m_comments_and_blobs.end();++it) {
        if (it->first=="comment")
          mcpl_hdr_add_comment(m_f,it->second.c_str());
        else
          mcpl_hdr_add_data(m_f,it->first.c_str(),it->second.size(),it->second.c_str());
      }

      if (m_opt_writedoubleprec)   mcpl_enable_doubleprec(m_f);
      if (m_opt_writepolarisation) mcpl_enable_polarisation(m_f);
      if (m_opt_writeuserflags) mcpl_enable_userflags(m_f);
      if (m_opt_universalweight) mcpl_enable_universal_weight(m_f,m_opt_universalweight);

      if (FrameworkGlobals::isForked()&&FrameworkGlobals::isParent()) {
        m_om = new MCPLOutputMerger(this);
        py::object pylauncher = pyextra::pyimport("G4Launcher").attr("getTheLauncher")();
        py::object pyhook = py::cast(m_om);
        pylauncher.attr("postmp_hook")(pyhook);
      }
    }

    std::string m_filename;
    bool m_opt_writedoubleprec;
    bool m_opt_writepolarisation;
    bool m_opt_writeuserflags;
    bool m_opt_writeonvolexit;
    KillStrategy m_opt_killstrategy;
    double m_opt_universalweight;
    bool m_initialised;
    bool m_closed;
    mcpl_outfile_t m_f;
    mcpl_particle_t m_p;
    MCPLOutputMerger * m_om;
    std::vector<std::pair<std::string,std::string>> m_comments_and_blobs;
  };

  void MCPLOutputMerger::merge() {
    m_sd->ensure_close_file();
    std::string filename = m_sd->filename();
    //Ok, we now have nprocs output files which we need to merge.
    unsigned nprocs = FrameworkGlobals::nProcs();
    for (unsigned i = 1; i < nprocs; ++i) {
      std::stringstream tmp;
      tmp << filename << "." << i << ".mcpl";
      std::string fn2 = tmp.str();

      printf("MCPLWriter: Merging output of proc%i into %s\n",i,filename.c_str());
      mcpl_merge_inplace(filename.c_str(), fn2.c_str());
      int r = remove(fn2.c_str());
      if (r)
        printf("MCPLWriter WARNING: Could not remove file after merging: %s\n",fn2.c_str());
    }
    printf("MCPLWriter: Done merging output from %i processes into file %s\n",nprocs,filename.c_str());
    mcpl_gzip_file(filename.c_str());
  }

  class MCPLWriter {
  private:
    std::string m_filename;
    bool m_opt_writedoubleprec;
    bool m_opt_writepolarisation;
    bool m_opt_writeuserflags;
    bool m_opt_writeonvolexit;
    KillStrategy m_opt_killstrategy;
    double m_opt_universalweight;
    BuilderPtr m_expr_builder;
    EP::Evaluator<bool> m_eval_filter;
    EP::Evaluator<EP::int_type> m_eval_flags;
    std::string m_expr_filter;
    std::string m_expr_flags;
  public:
    void setWriteDoublePrecision(bool b) {  m_opt_writedoubleprec = b; }
    void setWritePolarisation(bool b) {  m_opt_writepolarisation = b; }
    void setWriteUserFlags(bool b) {  m_opt_writeuserflags = b; }
    void setWriteOnVolExit(bool b) {  m_opt_writeonvolexit = b; }
    void setUniversalWeight(double w) {  m_opt_universalweight = w; }
    void setFilter(const char * expr) { m_expr_filter = expr; }
    void setUserFlags(const char * expr) { m_expr_flags = expr; }
    void setKillStrategy(KillStrategy ks) {  m_opt_killstrategy = ks; }
    void setKillStrategyFromString(const char* ks) {  setKillStrategy(ks_from_string(ks)); }

    MCPLWriter(const char* filename)
      : m_filename(filename),
        m_opt_writedoubleprec(false),
        m_opt_writepolarisation(false),
        m_opt_writeuserflags(false),
        m_opt_writeonvolexit(false),
        m_opt_killstrategy(KS_NEVER),
        m_opt_universalweight(0.0),
        m_expr_filter("true"),
        m_vols_all(false),
        m_sd(0)
    {
      if (m_filename.size()<5||strcmp(&m_filename.at(m_filename.size()-5),".mcpl")!=0)
        m_filename += ".mcpl";
    }
    ~MCPLWriter()
    {
      //don't delete m_sd here (G4SDManager owns)
    }
    void addVolume(const char * lvname)
    {
      std::string s = lvname;
      if (s=="*")
        m_vols_all = true;
      else
        m_vols.insert(lvname);
    }
    void inithook() {

      if (m_vols.empty()&&!m_vols_all)
        throw std::runtime_error("MCPLWriter: No volumes added!");

      //Setup filter:

      if (m_eval_filter.arg()|| m_expr_builder)
        throw std::runtime_error("inithook called more than once");

      m_expr_builder = std::make_shared<G4ExprParser::G4SteppingASTBuilder>();
      try {
        m_eval_filter = m_expr_builder->createEvaluator<bool>(m_expr_filter);
      } catch (EP::InputError& e) {
        printf("\nMCPLWriter ERROR: Invalid filter expression \"%s\"\n",m_expr_filter.c_str());
        printf("MCPLWriter ERROR: %s : %s\n\n",e.epType(),e.epWhat());
        if (m_sd)
          m_sd->abort();
        throw;
      }

      if (m_eval_filter.isConstant()&&!m_eval_filter())
        printf("MCPLWriter WARNING: Specified filter \"%s\" always evaluates to false\n",m_expr_filter.c_str());

      //Setup expression for user-flags:

      try {
        if (!m_expr_flags.empty())
          m_eval_flags = m_expr_builder->createEvaluator<EP::int_type>(m_expr_flags);
      } catch (EP::InputError& e) {
        printf("\nMCPLWriter ERROR: Invalid expression for user-flas \"%s\"\n",m_expr_flags.c_str());
        printf("MCPLWriter ERROR: %s : %s\n\n",e.epType(),e.epWhat());
        if (m_sd)
          m_sd->abort();
        throw;
      }

      if (m_eval_flags.arg())
        m_opt_writeuserflags = true;

      if (m_eval_flags.isConstant()) {
        printf("MCPLWriter WARNING: Specified user flag generator \"%s\" always evaluates to the same value (%lli)\n",
               m_expr_filter.c_str(),(long long)m_eval_flags());

        if (!m_eval_flags())
          m_eval_flags.setArg(0);
      }

      auto vols_unused = m_vols;

      //Check names of all logical volumes:
      auto rStore = G4RegionStore::GetInstance();
      for ( auto itR = rStore->begin(); itR != rStore->end(); ++itR ) {
        auto itRLV = (*itR)->GetRootLogicalVolumeIterator();
        auto nRV = (*itR)->GetNumberOfRootVolumes();
        while(nRV) {
          --nRV;
          addSDRecursively(*itRLV,vols_unused);
          ++itRLV;
        }
      }
      //TODO: Complain about any added volumes not used!!

      for (auto it = vols_unused.begin(); it!=vols_unused.end(); ++it)
        printf("MCPLWriter: Warning - specified volume \"%s\" not found in geometry\n",it->c_str());

      if (!m_sd)
        throw std::runtime_error("MCPLWriter: No volumes selected!");


    }

  private:
    std::set<std::string> m_vols;
    bool m_vols_all;
    MCPLSensitiveDetector * m_sd;
    std::vector<std::pair<std::string,std::string>> createCommentsAndBlobs() const
    {
      std::vector<std::pair<std::string,std::string>> out;
      out.emplace_back("comment","Created with the Geant4 MCPLWriter in the ESS/dgcode framework");
      //volume info:
      {
        std::stringstream tmp;
        tmp << "MPCLWriter volumes considered : ";
        if (m_vols_all) {
          tmp << "<all>";
        } else if (m_vols.empty()) {
          tmp << "<none>";
        } else {
          tmp << "[";
          for (auto it = m_vols.begin();it!=m_vols.end();++it) {
            if (it!=m_vols.begin())
              tmp << ", ";
            tmp << "'" << *it << "'";
          }
          tmp << "]";
        }
        out.emplace_back("comment",tmp.str());
      }

      {
        std::stringstream tmp;
        tmp << "MPCLWriter steps considered : ";
        if (m_opt_writeonvolexit)
          tmp << "<at-volume-exit>";
        else
          tmp << "<at-volume-entry>";
        out.emplace_back("comment",tmp.str());
      }

      //filter info:
      const bool unfiltered = m_eval_filter.isConstant()&&m_eval_filter();
      {
        std::stringstream tmp;
        tmp << "MPCLWriter write filter : ";
        if (unfiltered)
          tmp << "<unfiltered>";
        else
          tmp << "[" << m_expr_filter << "]";
        out.emplace_back("comment",tmp.str());
      }

      //User flag info:
      {
        std::stringstream tmp;
        tmp << "MPCLWriter user flags : ";
        if (m_opt_writeuserflags) {
          if (m_eval_flags.arg())
            tmp << "[" <<m_expr_flags<<"]";
          else
            tmp <<"<capture-existing-flags>";
        } else {
          tmp << "<disabled>";
        }
        out.emplace_back("comment",tmp.str());
      }

      {
        std::stringstream tmp;
        tmp << "MPCLWriter track kill strategy : ";
        if (m_opt_killstrategy == KS_NEVER)
          tmp << "<none>";
        else if (m_opt_killstrategy == KS_ALWAYS || (m_opt_killstrategy == KS_FILTERED && unfiltered))
          tmp << "<tracks-of-considered-steps>";
        else
          tmp << "<tracks-of-written-steps>";
        out.emplace_back("comment",tmp.str());
      }

      auto mod = pyextra::pyimport("G4Launcher");//must work since we use it in inithook()
      auto launcher = mod.attr("getTheLauncher")();
      auto pygeo = launcher.attr("getGeo")();
      auto pygen = launcher.attr("getGen")();
      const G4Interfaces::GeoConstructBase* geo(0);
      const G4Interfaces::ParticleGenBase* gen(0);
      if (pygeo)
        geo = pygeo.cast<const G4Interfaces::GeoConstructBase*>();
      if (pygen)
        gen = pygen.cast<const G4Interfaces::ParticleGenBase*>();
      std::string geo_name(geo ? geo->getName() : std::string("<unknown>"));
      std::string gen_name(gen ? gen->getName() : std::string("<unknown>"));
      {
        std::stringstream tmp;
        tmp << "ESS/dgcode geometry module : " << geo_name;
        out.emplace_back("comment",tmp.str());
      }
      {
        std::stringstream tmp;
        tmp << "ESS/dgcode generator module : " << gen_name;
        out.emplace_back("comment",tmp.str());
      }

      if (geo) {
        char * geodata(0);
        unsigned lgeodata(0);
        geo->serialiseParameters(geodata,lgeodata);
        out.emplace_back("ESS/dgcode_geopars",std::string(geodata,lgeodata));
        delete [] geodata;
      }

      if (gen) {
        char * gendata(0);
        unsigned lgendata(0);
        gen->serialiseParameters(gendata,lgendata);
        out.emplace_back("ESS/dgcode_genpars",std::string(gendata,lgendata));
        delete [] gendata;
      }

      return out;
    }

    void addSDRecursively(G4LogicalVolume *lv, std::set<std::string>& vols_unused ) {
      //Always add SD on selected logical volumes, even if already added on a
      //mother volume. This is to allow multiple MCPLWriter instances to
      //properly coexist (and one could imagine it to be faster during
      //simulation as well).

      if (m_vols_all || m_vols.find(lv->GetName())!=m_vols.end()) {
        //This LV is selected by user
        if (!m_sd) {
          //Must initialise sensitive detector. We make sure with a slightly nasty
          //static trick to avoid name clashes for sensitive detectors (G4 require
          //them to have unique names):
          static int nsd = 0;
          ++nsd;
          std::string sdname = "MCPLSensitiveDetector";
          if (nsd!=1) {
            std::stringstream tmp;
            tmp << sdname << "_nbr_" << nsd;
            sdname = tmp.str();
          }

          m_sd = new MCPLSensitiveDetector(sdname,
                                           m_filename,
                                           m_opt_writedoubleprec,
                                           m_opt_writepolarisation,
                                           m_opt_writeuserflags,
                                           m_opt_universalweight,
                                           m_opt_writeonvolexit,
                                           m_opt_killstrategy,
                                           m_expr_builder,
                                           m_eval_filter,
                                           m_eval_flags,
                                           createCommentsAndBlobs());
          G4SDManager::GetSDMpointer()->AddNewDetector(m_sd);
        }
        auto currentsd = lv->GetSensitiveDetector();
        if (!currentsd) {
          lv->SetSensitiveDetector(m_sd);
        } else if (currentsd!=m_sd) {
          if (auto currentsd_mcpl = dynamic_cast<MCPLSensitiveDetector*>(currentsd)) {
            currentsd_mcpl->addChild(m_sd);
          } else {
            m_sd->abort();
            throw std::runtime_error("MCPLWriter conflict: Volume has existing sensitive detector!\n");
          }
        }
        if (vols_unused.count(lv->GetName()))
          vols_unused.erase(lv->GetName());
      }

      //recurse to daughters:
      std::set<G4LogicalVolume*> checked;//local protection against repeat work
      int n = lv->GetNoDaughters();
      for (int i = 0; i<n; ++i) {
        auto dlv = lv->GetDaughter(i)->GetLogicalVolume();
        if (!checked.count(dlv)) {
          addSDRecursively(dlv,vols_unused);
          checked.insert(dlv);
        }
      }
    }

  };


}

PYTHON_MODULE( mod )
{
  using namespace G4MCPLWriter;

  py::class_<MCPLWriter,std::shared_ptr<MCPLWriter>>(mod,"MCPLWriter")
    .def( py::init<const char*>() )
    .def("addVolume",&MCPLWriter::addVolume)
    .def("setFilter",&MCPLWriter::setFilter)
    .def("setUserFlags",&MCPLWriter::setUserFlags)
    .def("setWriteOnVolExit",&MCPLWriter::setWriteOnVolExit)
    .def("setWriteDoublePrecision",&MCPLWriter::setWriteDoublePrecision)
    .def("setWritePolarisation",&MCPLWriter::setWritePolarisation)
    .def("setWriteUserFlags",&MCPLWriter::setWriteUserFlags)
    .def("setUniversalWeight",&MCPLWriter::setUniversalWeight)
    .def("setKillStrategy",&MCPLWriter::setKillStrategyFromString)
    .def("inithook",&MCPLWriter::inithook)
    ;

  py::class_<MCPLOutputMerger>(mod,"_MCPLOutputMerger")
    .def("__call__",&MCPLOutputMerger::merge)
    ;
}

