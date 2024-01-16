#include "GriffDataRead/GriffDataReader.hh"
#include "GriffFormat/Format.hh"
#include "EvtFile/FileReader.hh"
#include "EvtFile/FileWriter.hh"

#include <algorithm>
#include <vector>
#include <set>
#include <string>
#include <cstdio>
#include <cstdint>

int main(int argc,char** argv) {
  std::vector<std::string> args(argv+1, argv+argc);
  bool request_help( std::find(args.begin(), args.end(), "-h") != args.end()
                     || std::find(args.begin(), args.end(), "--help") != args.end() );
  if (request_help || args.size()<3 ) {
    printf("\nUsage:\n\n  %s GRIFFINPUT GRIFFOUTPUT EVENTIDS\n\n"
           "Here, GRIFFINPUT is an existing file and GRIFFOUTPUT is the new\n"
           "output file in which the extracted event specified by EVENTIDS\n"
           "should be placed. EVENTIDS should be a list of positive integers\n"
           "separated by spaces, specifying the indices of events to extract.\n"
           "\nExample, extract 1st and 100th event from file:\n\n"
           "  %s myfile.griff newfile.griff 0 99\n\n",
           argv[0],argv[0]);
    return request_help ? 0 : 1;
  }
  auto it = args.begin();
  std::string infile = *it++;
  std::string outfile = *it++;
  std::set<std::uint64_t> selected_evts;
  for (;it!=args.end();++it) {
    std::size_t pos;
    unsigned long long l = stoull(*it,&pos);
    if (pos==it->size()) {
      selected_evts.insert((std::uint64_t)l);
    } else {
      printf("ERROR: Invalid event id requested!\n");
      return 1;
    }
  }
  GriffDataReader dr(infile);
  EvtFile::FileWriter fw(GriffFormat::Format::getFormat(),outfile.c_str());
  if (!fw.ok()) {
    printf("ERROR: Problems opening requested output file\n");
    return 1;
  }
  std::vector<char> pending_shared_data;
  std::vector<char> tmp;
  std::uint64_t ntaken(0);
  std::uint64_t nseen(0);

  while (dr.loopEvents()) {
    auto evtid = dr.loopCount();
    if (!dr.verifyEventDataIntegrity()) {
      printf("ERROR: Data integrity check failed in input data on event #%llu\n",
             (unsigned long long)evtid);
      return 1;
    }
    if ( dr.eventIndexInCurrentFile()==0 && nseen ) {
      printf("ERROR: Extracting from multiple input files is currently not supported!\n");
      return 1;
    }
    ++nseen;
    if (nseen%10000==0&&nseen)
      printf("Processed %llu events\n",(unsigned long long)nseen);
    auto fr = dr.getRawFileReader();
    if (fr->nBytesSharedDataInEvent()) {
      fr->getSharedDataInEvent(tmp);
      if (!tmp.empty()) {
        pending_shared_data.reserve(pending_shared_data.size()+tmp.size());
        pending_shared_data.insert(pending_shared_data.end(),tmp.begin(),tmp.end());
      }
    }

    if (selected_evts.count(evtid)) {
      ++ntaken;
      selected_evts.erase(evtid);
      if (!pending_shared_data.empty())
        fw.writeDataDBSection(&pending_shared_data[0], pending_shared_data.size());
      if (fr->nBytesBriefData())
        fw.writeDataBriefSection(fr->getBriefData(), fr->nBytesBriefData());
      if (fr->nBytesFullData())
        fw.writeDataFullSection(fr->getFullData(), fr->nBytesFullData());
      fw.flushEventToDisk(fr->runNumber(),fr->eventNumber());//Call at end of each event
      pending_shared_data.clear();

      printf("Copied event #%llu to output file\n",(unsigned long long)evtid);
      if (selected_evts.empty()) {
        printf("No more events requested from input file - ending.\n");
        break;
      }
    }
  }

  fw.close();

  if (!ntaken) {
    //Best to threat this as an error, since the file doesn't even contain any
    //metadata like geometry parameters etc.
    printf("ERROR: No events selected from file (file has %llu events).\n",
           (unsigned long long)nseen);
    return 1;
  }

  if (!selected_evts.empty()) {
    printf("WARNING: Some requested events were not extracted (file has just %llu events).\n",
           (unsigned long long)nseen);
  }

  printf("Extracted %llu events from input file into new file %s\n",
         (unsigned long long)ntaken,outfile.c_str());

  return 0;
}

//TODO: unit test
