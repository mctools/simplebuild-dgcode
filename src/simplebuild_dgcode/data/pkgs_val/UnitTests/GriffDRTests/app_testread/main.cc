#include "GriffDataRead/GriffDataReader.hh"
#include "GriffDataRead/DumpObj.hh"
#include "Core/FindData.hh"
#include "Units/Units.hh"
#include "Core/FPE.hh"
#include <set>
#include <iostream>
#include <algorithm>
#include <random>
#include <tuple>

void test(bool b)
{
  //use test rather than assert to make sure it is triggered in release build (we could perhaps add an opt_assert to Core?)
  if (!b) {
    printf("ERROR: Test failed!\n");
    assert(false);
    exit(1);
  }
}

int test_seek(const char* datafile) {

  //Test (for event+track browsers for instance), the use-case of first running
  //through a file to establish a list of event indices and number of tracks,
  //and then subsequently seeking around based on those indices, to access track
  //data.
  GriffDataReader dr(datafile);
  std::vector<std::tuple<unsigned,unsigned,std::uint32_t,std::string> > tracks;//evtidx, trkidx, checksum, pdgname
  while (dr.loopEvents()) {
    for (unsigned trkidx = 0; trkidx < dr.nTracks(); ++trkidx) {
      tracks.push_back(std::make_tuple(dr.eventIndexInCurrentFile(),trkidx,dr.eventCheckSum(),dr.getTrack(trkidx)->pdgName()));
    }
  }

  auto eng = std::default_random_engine(123456789);
  std::shuffle(tracks.begin(),tracks.end(),eng);

  auto itE = tracks.end();
  for (auto it = tracks.begin();it!=itE;++it) {
    dr.seekEventByIndexInCurrentFile(std::get<0>(*it));
    test(dr.eventActive());
    test(dr.eventCheckSum()==std::get<2>(*it));
    test(dr.verifyEventDataIntegrity());
    test(std::get<1>(*it)<dr.nTracks());
    auto trk = dr.getTrack(std::get<1>(*it));
    test(trk->pdgName()==std::get<3>(*it));
  }

  return 0;
}


void print_track_tree(const GriffDataRead::Track* trk, const std::string& prefix)
{
  printf("%s+ trkid=%i (%f keV %s)\n",prefix.c_str(),trk->trackID(),trk->startEKin()/Units::keV,trk->pdgNameCStr());
  if (trk->isSecondary()) {
    test(trk->getParent()->trackID()==trk->parentID());
  }
  for (unsigned i=0;i<trk->nDaughters();++i) {
    const GriffDataRead::Track* daughter = trk->getDaughter(i);
    print_track_tree(daughter,prefix+"  ");
    test(daughter->trackID()==trk->getDaughterID(i));
    test(daughter->parentID()==trk->trackID());
  }
}

int main(int,char**)
{
  Core::catch_fpe();

  //setup 3 loops through file with 10 evts:
  std::string datafile = Core::findData("GriffDataRead","10evts_singleneutron_on_b10_full.griff");
  GriffDataReader dr(datafile.c_str(),3);
  if (!dr.eventActive()) {
    printf("Error: No events in input file %s\n",datafile.c_str());
    return 1;
  }

  //Test goToNextEvent:
  unsigned ievt(0);
  while(dr.eventActive()) {
    ++ievt;
    dr.goToNextEvent();
  }
  printf("goToNextEvent test: ievt=%i (expects 30)\n",ievt);

  //Test goToFirstEvent
  bool ok = dr.goToFirstEvent();
  if (!ok) {
    printf("Error: goToFirstEvent failed\n");
    return 1;
  }
  ievt = 0;
  while(dr.eventActive()) {
    ++ievt;
    dr.goToNextEvent();
  }
  printf("goToFirstEvent test: ievt=%i (expects 30)\n",ievt);

  //Test skipEvents across file loop boundary:
  ok = dr.goToFirstEvent();
  if (!ok) {
    printf("Error: goToFirstEvent failed\n");
    return 1;
  }
  ok = dr.skipEvents(2);
  if (!ok) {
    printf("Error: skipEvents within file failed\n");
    return 1;
  }
  ok = dr.skipEvents(12);
  if (!ok) {
    printf("Error: skipEvents across file failed\n");
    return 1;
  }
  ievt = 0;
  while(dr.eventActive()) {
    ++ievt;
    dr.goToNextEvent();
  }
  printf("skipEvents test: ievt=%i (expects 16)\n",ievt);

  //test goToNextFile:
  ok = dr.goToFirstEvent() && dr.goToNextFile();
  if (!ok) {
    printf("Error: goToFirstEvent + goToNextFile failed\n");
    return 1;
  }
  ievt = 0;
  while(dr.eventActive()) {
    ++ievt;
    dr.goToNextEvent();
  }
  printf("goToNextFile test: ievt=%i (expects 20)\n",ievt);

  //Test track<->track navigation from primary tracks and downwards to daughters
  //(silently testing parents along the way):
  printf("\nTesting track<->track navigation in all events:\n\n");
  dr.goToFirstEvent();
  ievt=0;
  while(dr.eventActive()) {
    ++ievt;
    printf("(ievt,run,evt,mode)=(%i,%i,%i,%s): ntrks=%i\n",ievt,dr.runNumber(),dr.eventNumber(),dr.eventStorageModeStr(),dr.nTracks());
    test(dr.primaryTrackBegin()!=dr.primaryTrackEnd());
    for (auto it = dr.primaryTrackBegin();it!=dr.primaryTrackEnd();++it) {
      test(it->isPrimary());
      print_track_tree(it, "  ");
    }
    dr.goToNextEvent();
  }

  //Finally, dump all object data in all events (only some of the loop'ed events
  //though to keep test log-file size reasonable)

  std::set<const GriffDataRead::Material*> materials;

  printf("\nNow dumping all data in selected events:\n\n");
  dr.goToFirstEvent();
  ievt = 0;
  while(dr.eventActive()) {
    std::cout<<"(ievt,seed,run,evt,mode)=("<<ievt<<","<<dr.seed()<<","<<dr.runNumber()<<","
             <<dr.eventNumber()<<","<<dr.eventStorageModeStr()<<"): ntrks="<<dr.nTracks()<<""<<std::endl;
    for (auto trk = dr.trackBegin();trk!=dr.trackEnd();++trk) {
      auto parent = trk->getParent();
      if (trk->isSecondary()&&(trk->parentID()!=parent->trackID()))
        return 1;

      printf("   +");GriffDataRead::dump(trk,true);printf("\n");

      auto itSegE = trk->segmentEnd();
      unsigned isegment=0;
      const GriffDataRead::Segment * prevseg(0);
      for (auto itSeg = trk->segmentBegin();itSeg!=itSegE;++itSeg) {
        test(itSeg->getTrack()==trk);
        test(itSeg->iSegment()==isegment);
        if (prevseg) {
          test(prevseg==itSeg->getPreviousSegment());
          test(prevseg->getNextSegment()==itSeg);
        }
        printf("      + segment%i=",isegment++);GriffDataRead::dump(*itSeg);printf("\n");

        unsigned nmats(itSeg->volumeDepthStored());
        for (unsigned imat=0;imat<nmats;++imat)
          materials.insert(itSeg->material(imat));

        auto itStepE = itSeg->stepEnd();
        unsigned istep(0), stepmax(itSeg->nStepsStored());
        const GriffDataRead::Step * prevstep(0);
        for (auto itStep = itSeg->stepBegin();itStep!=itStepE;++itStep) {
          test(itStep==itSeg->getStep(istep));
          test(itStep->getSegment()==itSeg);
          test(itStep->getTrack()==trk);
          test(itStep->iStep()==istep);
          if (prevstep) {
            test(prevstep==itStep->getPreviousStep());
            test(prevstep->getNextStep()==itStep);
          }
          //reduce the size of the test logfile a bit by not printing each and every step:
          if (istep<4||istep>stepmax-3||istep%20==0)
            {
              printf("         + step%i=",istep);GriffDataRead::dump(*itStep);printf("\n");
            }
          ++istep;
          prevstep = itStep;
        }
        test(itSeg->nStepsStored()==istep);
        prevseg = itSeg;
      }
      test(trk->nSegments()==isegment);
    }
    if (ievt>10&&ievt<27) {
      ievt+=4;
      dr.skipEvents(4);
    } else {
      ievt+=1;
      dr.goToNextEvent();
    }
  }

  printf("Found %u different materials:\n",unsigned(materials.size()));
  auto it = materials.begin();
  auto itE = materials.end();
  for (;it!=itE;++it) {
    printf("    +");
    GriffDataRead::dump(*it);
    printf("\n");

    unsigned nelem((*it)->numberElements());
    for(unsigned ielem=0;ielem<nelem;++ielem) {
      const GriffDataRead::Element * elem = (*it)->getElement(ielem);
      printf("       +");
      GriffDataRead::dump(elem);
      printf("\n");
      unsigned niso(elem->numberIsotopes());
      for (unsigned iiso=0;iiso<niso;++iiso) {
        printf("          +");
        GriffDataRead::dump(elem->getIsotope(iiso));
        printf("\n");
      }
    }
  }

  printf("ievt=%i\n",ievt);
  if (ievt!=30) {
    printf("Error: Did not find 3*10 events when running through the file 3 times!\n");
    return 1;
  }

  return test_seek(datafile.c_str());
}
