#include "DCSteppingAction.hh"
#include "G4Interfaces/FrameworkGlobals.hh"
#include "EvtFile/Defs.hh"
#include "Core/String.hh"
#include <cassert>
#include "Utils/Format.hh"
#include <algorithm>
#include <set>
#include <limits>
#include <stdexcept>
#include "DBMetaDataEntry.hh"
#include "Randomize.hh"
#include "G4Version.hh"

#ifdef G4MULTITHREADED
#  include "G4RunManager.hh"
#  include "G4MTRunManager.hh"
#endif

namespace G4DataCollectInternals {

  DCSteppingAction::DCSteppingAction(const char* outputFile, GriffFormat::Format::MODE mode, G4UserSteppingAction * otherAct)
    : G4UserSteppingAction(), m_mode(mode), m_otherAction(otherAct),
      m_stepFilter(0), m_stepKillFilter(0), m_doFilter(false),
      m_prevTrkId(INT_MAX), m_prevStepNbr(INT_MAX-1), m_prevVol(0),
      m_currentMetaDataIdx(EvtFile::INDEX_MAX),
      m_mgr(0), m_outputFile(outputFile)
  {
    //Todo: user should be able to change mode on the fly, and even be able to skip writing of event entirely.
  }

  DCSteppingAction::~DCSteppingAction()
  {
    delete m_stepFilter;
    delete m_stepKillFilter;
    delete m_mgr;
  }

  void DCSteppingAction::initMgr()
  {
#ifdef G4MULTITHREADED
    {
      auto runmgr = G4RunManager::GetRunManager();
      if (!runmgr)
        throw std::runtime_error("Logic error: Griff DCSteppingAction should not initialise before G4RunManager is created.");
      if (dynamic_cast<G4MTRunManager*>(G4RunManager::GetRunManager()))
        throw std::runtime_error("Griff Data collection detected usage of G4MTRunManager which is not supported!");
    }
#endif

    std::string extension(GriffFormat::Format::getFormat()->fileExtension());
    bool has_extension(Core::ends_with(m_outputFile,extension));
    if (FrameworkGlobals::isForked()) {
      std::string orig = m_outputFile;
      if (orig.size()>=extension.size()&&has_extension)
        orig.resize(orig.size()-extension.size());
      std::string tmp;
      Utils::string_format(tmp,"%s.%i",orig.c_str(),FrameworkGlobals::mpID());
      m_outputFile = tmp;
    } else {
      if (!has_extension)
        m_outputFile += extension;
    }
    m_mgr = new DCMgr(m_outputFile.c_str());
    if (m_stepFilter)
      m_stepFilter->initFilter();
    if (m_stepKillFilter)
      m_stepKillFilter->initFilter();
  }

  void DCSteppingAction::setMetaData(const std::string& ckey,const std::string& cvalue)
  {
    assert(!ckey.empty());
    m_pendingMetaData.push_back(std::make_pair(ckey,cvalue));
  }

  void DCSteppingAction::setBasicMetaData()
  {
    //G4Version
    std::vector<std::string> strs;
    Core::split(strs,G4Version," ");
    if ( strs[2]=="[MT]$" ) {
      strs[2]="$";
      strs[1] += "_MT";
    }
    assert(strs.size()==3&&strs[0]=="$Name:"&&strs[2]=="$");
    setMetaData("G4Version",strs[1].c_str());
    std::string tmp;
    Utils::string_format(tmp,"%i",G4VERSION_NUMBER);
    setMetaData("G4VersionNumber",tmp.c_str());
    setMetaData("G4Date",G4Date);
    //GRIFF Mode:
    if (m_mode==GriffFormat::Format::MODE_FULL) setMetaData("GriffMode","FULL");
    else if (m_mode==GriffFormat::Format::MODE_REDUCED) setMetaData("GriffMode","REDUCED");
    else if (m_mode==GriffFormat::Format::MODE_MINIMAL) setMetaData("GriffMode","MINIMAL");
    else { assert(false); }
    //Random engine name:
    setMetaData("RandEngine",CLHEP::HepRandom::getTheEngine()->name());
    //Data libraries (at least, the name they point to)
    std::vector<std::string> envname;
    envname.push_back("G4LEDATA");
    envname.push_back("G4LEVELGAMMADATA");
    envname.push_back("G4NEUTRONHPDATA");
#if G4VERSION_NUMBER < 1100
    envname.push_back("G4NEUTRONXSDATA"); //Observed absent in 11.0.3, and present in 10.4.3
#endif
    envname.push_back("G4PIIDATA");
    envname.push_back("G4RADIOACTIVEDATA");
    envname.push_back("G4REALSURFACEDATA");
    envname.push_back("G4SAIDXSDATA");
#if G4VERSION_NUMBER >= 1100
    //Introduced at some point between 10.4.3 and 11.0.3:
    envname.push_back("G4ENSDFSTATEDATA");
    envname.push_back("G4INCLDATA");
    envname.push_back("G4PARTICLEXSDATA");
    envname.push_back("G4ABLADATA");
#endif

    for (unsigned i=0;i<envname.size();++i) {
      const char * env = getenv(envname.at(i).c_str());
      std::string tmpkey("datadir/");
      tmpkey+=envname.at(i);
      if (env) {
        std::vector<std::string> parts;
        Core::split_noempty(parts,env,"/");
        std::string tmpmd;
        if (!parts.empty())
          tmpmd = parts.back();
        //In conda-forge packages, but not in standalone G4 installations, the
        //data dirs are "xxx" instead of "G4xxx". For consistency (esp. of
        //reference log files), we add the prefix manually if missing:
        std::string compat_prefix("G4");
        if ( !tmpmd.empty() && !Core::starts_with( tmpmd, compat_prefix) )
          tmpmd = ( compat_prefix + tmpmd );
        setMetaData(tmpkey,tmpmd.c_str());
      } else {
        setMetaData(tmpkey,"");
      }
    }
  }

  void DCSteppingAction::UserSteppingAction(const G4Step*step)
  {
    if (m_otherAction) {
      m_otherAction->UserSteppingAction(step);
    }
    if (!m_mgr)
      initMgr();

    //All the stuff in the following section is to detect and correct the rare
    //case where G4 has buggy atVolEdge flags:
    int isNewVolOnSameTrack(-1);//-1==unknown, 0=no, 1=yes;
    auto track = step->GetTrack();
    int trkid = track->GetTrackID();
    int stepNbr = track->GetCurrentStepNumber();
    G4VPhysicalVolume* vol = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume(0);
    assert(vol);
    if (m_prevTrkId==trkid&&m_prevStepNbr+1==stepNbr)
      isNewVolOnSameTrack = (vol==m_prevVol?0:1);
    m_prevVol=vol; m_prevTrkId=trkid; m_prevStepNbr=stepNbr;

    if (m_doFilter) {
      if (m_stepKillFilter && m_stepKillFilter->filterStep(step) == m_stepKillFilter->negated() ) {
        track->SetTrackStatus(fStopAndKill);
        return;
      }
      if (m_stepFilter && m_stepFilter->filterStep(step) == m_stepFilter->negated() )
        return;
    }
    //passed any filters so record:
    unsigned nstepsprev = m_steps.size();
    DCStepData * newstep = mempoolGetStepObject();
    m_steps.push_back(newstep);
#ifdef GRIFF_EXTRA_TESTS
    assert(!newstep->valid());
    for (unsigned i=1;i<m_steps.size();++i) {
      assert(m_steps.at(i-1)->valid());
    }
#endif
    G4DataCollectInternals::DCStepData * prevstep = ( (nstepsprev>0&&isNewVolOnSameTrack!=-1) ? m_steps[nstepsprev-1] : 0 );
    newstep->set(step,*m_mgr,isNewVolOnSameTrack,prevstep);


#ifdef GRIFF_EXTRA_TESTS
    for (unsigned i=0;i<m_steps.size();++i) {
      assert(m_steps.at(i)->valid());
    }
#endif
  }

  struct DCSteppingAction::Track_
  {
    Track_(G4int trkId_,
           unsigned stepBegin_)
      : trkId(trkId_),
        nSegments(0),
        creatorProcIdx(EvtFile::INDEX_MAX),
        stepBegin(stepBegin_),
        stepEnd(EvtFile::INDEX_MAX) {}
    G4int trkId;
    unsigned nSegments;
    unsigned segmentsBegin;
    EvtFile::index_type creatorProcIdx;
    unsigned stepBegin;
    unsigned stepEnd;
  };

  bool compareSteps(const DCStepData* lhs,const DCStepData* rhs)
  {
#ifdef GRIFF_EXTRA_TESTS
    assert(lhs->valid()&&rhs->valid());
    lhs->extraTests();
    rhs->extraTests();
#endif
    //sort by trackid first, step number second
    return lhs->trkId==rhs->trkId ? lhs->stepNbr<rhs->stepNbr : lhs->trkId<rhs->trkId;
  }

  void DCSteppingAction::EndOfEventAction(const G4Event*)
  {
    if (!m_mgr)//check here as well, in case 1st event had no tracks.
      initMgr();

    //Prepare steps:
    std::sort(m_steps.begin(),m_steps.end(),compareSteps);//cheap, just swapping order of pointers

    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    //// Run through steps once to:                               ////
    ////          * find tracks and number of segments            ////
    ////          * establish mother-daughter relationships.      ////
    ////          * register unique pdg codes in DB               ////
    ////          * creator processes                             ////
    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////

    // Track related data structures:
    std::vector<Track_> tracks;
    tracks.reserve(std::min<unsigned>(1024,4*(m_steps.size()/12)));//fixme: cache tracks object?
    std::map<G4int,std::set<G4int> > trkid2daughters;//todo: benchmark unordered_map.
    // unsigned ndaughterfields_tot(0);
    std::set<G4int> unique_pdgcodes;
    G4int pdgcode_prev = std::numeric_limits<G4int>::max();
    EvtFile::index_type prev_volidx(EvtFile::INDEX_MAX);
    EvtFile::index_type volidx(EvtFile::INDEX_MAX);
    unsigned nSegments = 0;
    unsigned iSegments = 0;
    G4int trkId_prev(-9999);
    G4int stepNbr_prev(-9999);
    unsigned currentSegmentBegin = EvtFile::INDEX_MAX;
    // Run through steps and fill the track related data structures:
    const unsigned nsteps = m_steps.size();
    for (unsigned istep = 0; istep<nsteps;++istep) {
      DCStepData & s = *(m_steps[istep]);
      bool first = tracks.empty();
      bool newtrack(first || s.trkId!=trkId_prev);
      if (newtrack) {
        if (!first) {
          //finish up previous track:
          tracks.back().stepEnd = istep;
          tracks.back().nSegments = nSegments;
          nSegments = 0;
          stepNbr_prev = -9999;
        }
        tracks.push_back(Track_(s.trkId,istep));//create new track
        trkId_prev=s.trkId;
        tracks.back().segmentsBegin = iSegments;
        if (s.creatorProcessName)
          tracks.back().creatorProcIdx = m_mgr->dbProcNames.getIndex(*(s.creatorProcessName));
        if (pdgcode_prev != s.pdgcode || first) {//pdg code
          unique_pdgcodes.insert(s.pdgcode);
          pdgcode_prev = s.pdgcode;
        }
        if (s.parentId!=0) {
          //track has a parent (i.e. it is a secondary) so we fill the trkid => daughterids map
          auto it = trkid2daughters.find(s.parentId);
          if (it == trkid2daughters.end()) {
            std::set<G4int> daughters;
            daughters.insert(s.trkId);
            // ++ndaughterfields_tot;
            trkid2daughters[s.parentId] = daughters;//inefficient std::set copy?
          } else {
            it->second.insert(s.trkId);
            // auto res = it->second.insert(s.trkId);
            // if (res.second)
            //   ++ndaughterfields_tot;
          }
        }
      }//endif new-track

      //define segments by volume changes (or when steps are omitted due to filtering).

      if (s.touchableEntry || s.stepNbr!=stepNbr_prev+1) {
        //step is first on track, it is the first in a new volume, or it is the
        //first after omitted steps, thus defining a new segment:
#ifdef GRIFF_EXTRA_TESTS
        if (s.touchableEntry)
          s.touchableEntry->extraTests();
#endif
        if (currentSegmentBegin!=EvtFile::INDEX_MAX)
          m_steps[currentSegmentBegin]->segmentEnd = istep;
        currentSegmentBegin = istep;
        volidx = s.touchableEntry ? m_mgr->dbTouchables.getIndex(s.touchableEntry) : prev_volidx;
#ifdef GRIFF_EXTRA_TESTS
        assert(volidx!=EvtFile::INDEX_MAX);
        if (s.stepNbr==stepNbr_prev+1&&(volidx!=prev_volidx?1:0)!=(s.preStep.atVolEdge?1:0)) {
          const char * p = FrameworkGlobals::printPrefix();
          std::cout<<p<<"ERROR: Failed atVolEdge sanity check. Seed is "<<FrameworkGlobals::currentEvtSeed()<<std::endl;
          std::cout<<p<<"Info about failing step:"<<std::endl;
          std::cout<<p<<"        trkId    = "<<s.trkId<<std::endl;
          std::cout<<p<<"        parentId = "<<s.parentId<<std::endl;
          std::cout<<p<<"        stepNbr  = "<<s.stepNbr<<std::endl;
          std::cout<<p<<"        pdgcode  = "<<s.pdgcode<<std::endl;
          std::cout<<p<<"        newtrack = "<<(newtrack?"yes":"no")<<std::endl;
          std::cout<<p<<"        volname  = "<<s.touchableEntry->name()<<std::endl;
          std::cout<<p<<"        volidx      = "<<volidx<<std::endl;
          std::cout<<p<<"        prev_volidx = "<<prev_volidx<<std::endl;
          std::cout<<p<<"        s.preStep.atVolEdge   = "<<s.preStep.atVolEdge<<std::endl;
          std::cout<<p<<"        s.postStep.atVolEdge  = "<<s.postStep.atVolEdge<<std::endl;
          std::cout<<p<<"        newtrack = "<<(newtrack?"yes":"no")<<std::endl;
          assert(false);
        }
#endif
        prev_volidx=volidx;
        // New segment:
        ++nSegments;
        ++iSegments;
      }
      s.volIdx = volidx;
      stepNbr_prev = s.stepNbr;
    }
    //Finish up the very last track:
    if (!tracks.empty()) {
      tracks.back().stepEnd = nsteps;
      tracks.back().nSegments = nSegments;
      //nSegments=0;
      if (currentSegmentBegin!=EvtFile::INDEX_MAX)
        m_steps[currentSegmentBegin]->segmentEnd = nsteps;
    }


    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    //// MetaData                                                 ////
    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    if (m_currentMetaDataIdx==EvtFile::INDEX_MAX) {
      //add G4 version, name of random gen, etc. to metadata.
      setBasicMetaData();
    }
    if (!m_pendingMetaData.empty()) {
      auto itE = m_pendingMetaData.end();
      for (auto it = m_pendingMetaData.begin();it!=itE;++it)
        m_currentMetaData[it->first]=it->second;
      auto dbentry = new DBMetaDataEntry(m_currentMetaData,m_mgr);
      dbentry->ref();
      m_currentMetaDataIdx = m_mgr->dbMetaData.getIndex(dbentry);
      dbentry->unref();
      m_pendingMetaData.clear();
    }
    assert(m_currentMetaDataIdx!=EvtFile::INDEX_MAX);

    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    //// Write out track info in the brief data section           ////
    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////

    auto itTrackE=tracks.end();
    EvtFile::FileWriter& fw = m_mgr->fileWriter;
    //This is also where we embed the event seed!
    fw.writeDataBriefSection((std::uint64_t)FrameworkGlobals::currentEvtSeed());
    fw.writeDataBriefSection(m_currentMetaDataIdx);
    fw.writeDataBriefSection((std::uint32_t)tracks.size());
    fw.writeDataBriefSection((std::uint32_t)m_mode);//could be squeezed into the track size word and hope we had <1e9 tracks

    for (auto itTrack=tracks.begin();itTrack!=itTrackE;++itTrack) {
      Track_ & trk = *itTrack;
      DCStepData & step = * (m_steps[trk.stepBegin]);
      auto itDaughters = trkid2daughters.find(trk.trkId);
      unsigned nDaughters = itDaughters==trkid2daughters.end() ? 0 : itDaughters->second.size();

      fw.writeDataBriefSection((int32_t)trk.trkId);
      fw.writeDataBriefSection((int32_t)step.pdgcode);
      fw.writeDataBriefSection((float)step.preStep.weight);
      fw.writeDataBriefSection((EvtFile::index_type)trk.creatorProcIdx);
      fw.writeDataBriefSection((int32_t)step.parentId);
      fw.writeDataBriefSection((std::uint32_t)trk.nSegments);
      fw.writeDataBriefSection((std::uint32_t)nDaughters);
      assert(trk.nSegments>0);
      if (nDaughters)
        for (auto itD = itDaughters->second.begin();itD!=itDaughters->second.end();++itD)
          fw.writeDataBriefSection((int32_t)*itD);
    }

    //Register pdg codes so we get their properties written out:
    for(auto itPDG = unique_pdgcodes.begin(), itPDGE = unique_pdgcodes.end();itPDG!=itPDGE;++itPDG) {
      m_mgr->dbPDGCodes.registerPDGCode(*itPDG);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////
    //// Write out segment info in the brief data section and step info in the full data section ////
    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////

    //prev_volidx = EvtFile::INDEX_MAX;
    for (auto itTrack=tracks.begin();itTrack!=itTrackE;++itTrack) {
      Track_ & trk = *itTrack;
      for (unsigned istep = trk.stepBegin;istep<trk.stepEnd;) {
        DCStepData & step = *(m_steps[istep]);
        assert(step.segmentEnd!=EvtFile::INDEX_MAX);//due to actions further down in the loop, we should
                                   //only see the steps at the start of a segment up here.
        assert(step.volIdx!=EvtFile::INDEX_MAX);//all steps should have this info by now

        //we have a segment with steps running from *itStep to step.segmentEnd:
        unsigned nsegsteps = step.segmentEnd - istep;
        assert(nsegsteps>0);//otherwise how did the segment get defined...
        assert(nsegsteps<INT32_MAX);//the sign bit is reserved for other purposes...
        fw.writeDataBriefSection((double)step.preStep.time);//time_start
        fw.writeDataBriefSection((double)step.preStep.eKin);//ekin_start

        assert(step.volIdx <= 0x1FFFFFFF/*536870911*/);//three upper bits can be abused!
        EvtFile::index_type volinfo = step.volIdx;//"& 0x3FFFFFFF" not needed, as per assert above
        if (step.preStep.atVolEdge) volinfo |= 0x40000000;
        DCStepData & laststep = *(m_steps[step.segmentEnd-1]);
        if (laststep.postStep.atVolEdge) volinfo |= 0x80000000;
        bool nextWasFiltered(false);
        if (step.segmentEnd!=trk.stepEnd&&laststep.stepNbr + 1 != m_steps[step.segmentEnd/*first on next segment*/]->stepNbr) {
          //This is a segment whose last step due to filtering does not
          //immediately preceede the first step of the next segment
          volinfo |= 0x20000000;
          nextWasFiltered = true;
        }
        static_assert( (0x80000000|0x40000000|0x20000000|0x1FFFFFFF) == 0xFFFFFFFF);
        static_assert( (0x80000000&0x40000000&0x20000000&0x1FFFFFFF) == 0x00000000);
        static_assert(sizeof(EvtFile::index_type)>=4);
        fw.writeDataBriefSection(volinfo);

        if (m_mode==GriffFormat::Format::MODE_MINIMAL)
          fw.writeDataBriefSection(-((int32_t)nsegsteps));//minimal mode: put -nsegsteps where other modes put a step position
        else {
          assert(fw.sizeFullDataSection()<INT32_MAX);
          fw.writeDataBriefSection((int32_t)fw.sizeFullDataSection());
        }
        unsigned segmentEnd = step.segmentEnd;
        double edep(0), edep_nonion(0), stepLength(0);
        std::uint32_t lastStepStatus(fUndefined);
        if (m_mode!=GriffFormat::Format::MODE_MINIMAL)
          {
            //put nsegsteps info in step section:
            fw.writeDataFullSection((std::uint32_t)nsegsteps);//nsegsteps_orig
            fw.writeDataFullSection((std::uint32_t)(m_mode==GriffFormat::Format::MODE_FULL ? nsegsteps : 1));//nsegsteps_stored
            static_assert(GriffFormat::Format::SIZE_STEPHEADER==2*sizeof(std::uint32_t));
          }
        if (m_mode==GriffFormat::Format::MODE_REDUCED) {
          //prestep from the first step
          assert(istep<nsteps);
          step.preStep.write(fw);
        }
        unsigned nsteps_onsegment = 0;
        for (;istep!=segmentEnd;++istep) {
          DCStepData & sstep = *(m_steps[istep]);
          edep += sstep.eDep;
          edep_nonion += sstep.eDepNonIonizing;
          stepLength += sstep.stepLength;
          lastStepStatus = sstep.stepStatus;
          ++nsteps_onsegment;
          if (m_mode!=GriffFormat::Format::MODE_FULL)
            continue;
          assert(istep<nsteps);
          sstep.preStep.write(fw);
          fw.writeDataFullSection(float(sstep.eDep));
          fw.writeDataFullSection(float(sstep.eDepNonIonizing));
          fw.writeDataFullSection(float(sstep.stepLength));
          fw.writeDataFullSection(std::uint32_t(sstep.stepStatus));//waste of 3.5 bytes...
          static_assert(GriffFormat::Format::SIZE_STEPOTHERPART==3*sizeof(float)+sizeof(std::uint32_t));
          if (istep+1==segmentEnd) {
            //Only the last step needs postStep, the other ones can read the first part of the next steps.
            assert(istep<nsteps);
            sstep.postStep.write(fw);
          }
        }
        if (m_mode==GriffFormat::Format::MODE_REDUCED) {
          //sum of all edep's:
          fw.writeDataFullSection((float)edep);
          fw.writeDataFullSection((float)edep_nonion);
          fw.writeDataFullSection(float(stepLength));
          fw.writeDataFullSection(std::uint32_t(nsteps_onsegment == 1 ? lastStepStatus : std::uint32_t(fUndefined)));//step status undefined for coalesced steps
          static_assert(GriffFormat::Format::SIZE_STEPOTHERPART==3*sizeof(float)+sizeof(std::uint32_t));
          //poststep from the last step

          assert(istep==segmentEnd);
          assert(istep-1<nsteps);
          m_steps[istep-1]->postStep.write(fw);
        }
        //finish up the segment info:
        fw.writeDataBriefSection((float)edep);
        fw.writeDataBriefSection((float)edep_nonion);

        //Normally only the last segment on the track needs time_end and
        //ekin_end, since the other ones can read the first part of the next
        //segments. Unless of course segments are disconnected due to
        //filtering, in which case it is needed as well:
        assert(trk.stepEnd!=EvtFile::INDEX_MAX);
        if (istep==trk.stepEnd||nextWasFiltered)
          {
            fw.writeDataBriefSection((double)m_steps[istep-1]->postStep.time);//time_end
            fw.writeDataBriefSection((double)m_steps[istep-1]->postStep.eKin);//ekin_end
          }
      }
    }

    m_mgr->flushEventToDisk();
    clearSteps();

    //A few compile-time sanity checks:
    static_assert(sizeof(double)==8);
    static_assert(sizeof(float)==4);
    //These should not be strictly necessary:
    static_assert(sizeof(unsigned)==4);
    static_assert(sizeof(int)==4);
  }

}
