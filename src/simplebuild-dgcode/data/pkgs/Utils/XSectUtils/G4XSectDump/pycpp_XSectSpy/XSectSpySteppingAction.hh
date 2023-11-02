#ifndef pycpp_XSectSpy_XSectSpySteppingAction_hh
#define pycpp_XSectSpy_XSectSpySteppingAction_hh

#define protected public
#include "G4VDiscreteProcess.hh"
#undef protected
#include "G4Interfaces/FrameworkGlobals.hh"
#include "Utils/Format.hh"
#include "G4UserSteppingAction.hh"
#include "G4Track.hh"
#include "G4ProcessManager.hh"
#include "Core/File.hh"
#include "Core/String.hh"
#include <algorithm>
#include <set>
#include <string>
#include <sstream>
#include <cstdlib>//getenv

class XSectSpySteppingAction : public G4UserSteppingAction
{
  static std::string sm_lastWrittenFileName;
  static std::string sm_lastG4MaterialPrint;
  std::string m_physicslist_name;
  int m_limit;
public:
  static const char * lastWrittenFile() { return sm_lastWrittenFileName.c_str(); }
  static const char * lastG4MaterialPrinted() { return sm_lastG4MaterialPrint.c_str(); }

  XSectSpySteppingAction(const char* physicslist_name, int limit_nfiles = -1)
    : G4UserSteppingAction(),
      m_physicslist_name(physicslist_name),
      m_limit(limit_nfiles)
  {
    if (FrameworkGlobals::isForked()&&FrameworkGlobals::isChild())
      printf("%s XSectSpy: WARNING: Will not create cross-section files from child processes\n",FrameworkGlobals::printPrefix());
  }
  virtual ~XSectSpySteppingAction() {}

  mutable std::set<std::pair<const G4ParticleDefinition*, const G4Material*> > m_done;

  void createCrossSectionData(G4Track *track)
  {
    if (m_limit==0)
      return;
    auto pd = track->GetDynamicParticle()->GetParticleDefinition();
    auto mat = track->GetMaterial();
    auto key = std::pair<const G4ParticleDefinition*, const G4Material*>(pd,mat);
    if (m_done.count(key))
      return;
    m_done.insert(key);

    if (m_limit>0)
      --m_limit;

    std::ostringstream sss;
    sss << *mat;
    sm_lastG4MaterialPrint = sss.str();

    std::string outfile;
    std::string safematname = mat->GetName();
    Core::replace(safematname, "/", "%%");

    Utils::string_format(outfile, "xsects_discreteprocs_%s__%s__%s.txt",
                         pd->GetParticleName().c_str(),
                         safematname.c_str(),
                         m_physicslist_name.c_str());
    if (Core::file_exists(outfile))
      printf("%s XSectSpy: WARNING: Overwriting existing cross-section file %s\n",FrameworkGlobals::printPrefix(),outfile.c_str());
    else
      printf("%s XSectSpy: Writing X-section file %s\n",FrameworkGlobals::printPrefix(),outfile.c_str());

    FILE * pFile = fopen (outfile.c_str(),"w");
    assert(pFile);


    fprintf(pFile,"#FileType: Geant4 XSECTSPY DATA\n");
    fprintf(pFile,"#PhysicsList: %s\n",m_physicslist_name.c_str());
    fprintf(pFile,"#ParticleName: %s\n",pd->GetParticleName().c_str());
    fprintf(pFile,"#ParticleCode: %i\n",pd->GetPDGEncoding());
    fprintf(pFile,"#ParticleMass[MeV/c^2]: %g\n",pd->GetPDGMass()/CLHEP::MeV);
    fprintf(pFile,"#ParticleCharge: %f\n",pd->GetPDGCharge());
    fprintf(pFile,"#Material: %s\n",mat->GetName().c_str());
    fprintf(pFile,"#MaterialDensity [g/cm3]: %g\n",mat->GetDensity()/(CLHEP::gram/CLHEP::cm3));
    fprintf(pFile,"#MaterialTemperature [K]: %g\n",mat->GetTemperature()/CLHEP::kelvin);
    fprintf(pFile,"#MaterialPressure [bar]: %g\n",mat->GetPressure()/CLHEP::bar);
    const double atomic_weight_in_amu = (mat->GetDensity()/mat->GetTotNbOfAtomsPerVolume()) / CLHEP::amu;
    fprintf(pFile,"#MaterialAverageAtomicWeight [amu]: %g\n",atomic_weight_in_amu);
    fprintf(pFile,"#NAtomsPerVolume [1/cm3]: %g\n",mat->GetTotNbOfAtomsPerVolume()/(1/CLHEP::cm3));
    double conversion_factor = 1.0/( CLHEP::mm * CLHEP::barn * mat->GetTotNbOfAtomsPerVolume() );
    fprintf(pFile,"#ConvertXSectBarnToMeanFreePathMilliMeter: %g\n",conversion_factor);//k for "mfp = k/xsect"
    fprintf(pFile,"#EnergyUnit: eV\n");
    fprintf(pFile,"#XSectUnit: barn\n");

    std::set<double> energies;

    //For simplicity we use a few environment variables to allow power users to tune query granularity (todo: document in query script help and wiki)
    double nsample=50.0;
    double nsample_user = getenv("G4XSECTSPY_NSAMPLE") ? atof(getenv("G4XSECTSPY_NSAMPLE")) : 0.0;
    if (nsample_user) {
      nsample = nsample_user;
      printf("%s XSectSpy: As per G4XSECTSPY_NSAMPLE environment variable I am setting nsample to %g\n",FrameworkGlobals::printPrefix(),nsample);
    }
    double logdeltae=0.005;
    double logdeltae_user = getenv("G4XSECTSPY_LOGDELTAE") ? atof(getenv("G4XSECTSPY_LOGDELTAE")) : 0.0;
    if (logdeltae_user) {
      logdeltae = logdeltae_user;
      printf("%s XSectSpy: As per G4XSECTSPY_LOGDELTAE environment variable I am setting logdeltae to %g\n",FrameworkGlobals::printPrefix(),logdeltae);
    }
    for (double exponent=-9;exponent<10;exponent+=logdeltae)
      energies.insert(pow(10.0,exponent)*CLHEP::eV);

    std::vector<double> totals;

    auto pmanager = pd->GetProcessManager();
    assert(pmanager);
    auto pv = pmanager->GetProcessList();
    unsigned iprinted(0);
    auto size_pv = pv->size();
    for (decltype(size_pv) i=0;i<size_pv;++i) {
      auto pp = (*pv)[i];
      auto p = dynamic_cast<G4VDiscreteProcess *>(pp);
      if (!p)
        continue;

      if (!pmanager->GetProcessActivation(p))
        continue;

      bool printed_header(false);
      unsigned ie(0);
      //auto itprev = energies.end();
      //      int iii=0;

      //prune energy points too closely spaced
      std::set<double> energies_pruned;
      double eprev(1.0e99);
      for (auto it = energies.begin();it!=energies.end();++it) {
        const double e=*it;
        if (e-eprev>1e-13*CLHEP::eV)
          energies_pruned.insert(energies_pruned.end(),e);
        eprev=e;
      }
      energies.swap(energies_pruned);
      totals.resize(energies.size(),0);

      for (auto it = energies.begin();it!=energies.end();++it,++ie) {
        double e=*it;
        G4ForceCondition fc;
        auto ittmp = it;
        double emin = (it==energies.begin()?e:0.5*(e+*(--ittmp)));
        ittmp = it;
        ++ittmp;
        double emax = (ittmp==energies.end()?e:0.5*(e+*(ittmp)));
        double de=(emax-emin)/nsample;
        double mfp = 0;
        unsigned nsample_used(0);
        for (double esample = emin; esample<emax-0.5*de;esample+=de) {
          track->SetKineticEnergy(esample);
          double mfp1=p->GetMeanFreePath(*track,0,&fc);
          if (mfp1==DBL_MAX)
            continue;
          mfp += mfp1;
          ++nsample_used;
        }
        if (!nsample_used)
          continue;
        mfp/=nsample_used;
        // double mfp = p->GetMeanFreePath(*track,0,&fc);
        // if (mfp==DBL_MAX)
        //   continue;
        if (!printed_header) {
          printed_header=true;
          fprintf(pFile,"#Process %i: %s\n",iprinted++,pp->GetProcessName().c_str());
        }
        assert(mfp>0);
        double xsect=(1.0/(mfp*mat->GetTotNbOfAtomsPerVolume()));
        fprintf(pFile,"%.12g %.12g\n",e/CLHEP::eV,xsect/CLHEP::barn);
        totals.at(ie)+=xsect;
      }
    }
    fprintf(pFile,"#Process -: Total\n");
    unsigned ie(0);
    if (!totals.empty()) {//could be empty if there are no processes for the particle
      for (auto it = energies.begin();it!=energies.end();++it,++ie) {
        if (totals.at(ie))
          fprintf(pFile,"%.12g %.12g\n",*it/CLHEP::eV,totals.at(ie) / CLHEP::barn);
      }
    }


    fclose(pFile);
    sm_lastWrittenFileName = outfile;
  }

  //create x-sect for for each particle definition + material pointer (apart from galactic + world)

  void UserSteppingAction(const G4Step*step)
  {
    if (FrameworkGlobals::isForked()&&FrameworkGlobals::isChild())
      return;
    auto track = step->GetTrack();

    double ekin_orig = track->GetKineticEnergy();
    createCrossSectionData(track);

    if (track->GetKineticEnergy()!=ekin_orig)
      track->SetKineticEnergy(ekin_orig);
  }
};

std::string XSectSpySteppingAction::sm_lastWrittenFileName = "";
std::string XSectSpySteppingAction::sm_lastG4MaterialPrint = "";


#endif
