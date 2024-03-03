//We can do the shist browser with or without python. Choosing without python,
//since it saves >5seconds of compilation time!
#define MCPLEXTRA_AVOID_USING_PYTHON

#ifndef MCPLEXTRA_AVOID_USING_PYTHON
#  include "Core/Python.hh"
#endif
#include "MCPLExtra/HistCreate.hh"
#include "Core/String.hh"
#include "Core/File.hh"
#include <list>
#include <algorithm>
#include <string>
#include <cstdio>
#include <cstring>
#include <cstdio>
#ifdef MCPLEXTRA_AVOID_USING_PYTHON
#  include <cstdlib>
#  include <stdexcept>
#  include "sys/wait.h"
#  include <iostream>
#  include "Utils/Cmd.hh"
#endif

int usage(char**argv,const char* errmsg = 0) {

  const char * pn = strrchr(argv[0], '/');
  pn = pn ? pn + 1 : argv[0];
  if (errmsg) {
    printf("Bad usage: %s\n\n",errmsg);
    printf("Run %s --help for more information about correct usage.\n",pn);
    return 1;
  }
  printf("Usage:\n"
         "\n"
         "  %s [-n] MCPLFILE [PLOTEXPR] [where CONDEXPR]\n"
         "\n"
         "MCPLFILE is the name of an input file in MCPL format which should be examined.\n"
         "To ease working with very large files, one can optionally limit the number of\n"
         "particles loaded from the file by prefixing the filename with a colon and the\n"
         "desired limit like this: myfile.mcpl.gz:1000\n"
         "\n"
         "If PLOTEXPR is provided, it will be evaluated for each particle and the resul-\n"
         "ting distribution of values will be put into a 1D histogram in mcpl.shist and\n"
         "a graphical window will be launched to display it (specify -n to avoid launch-\n"
         "ing the window). Alternatively, if PLOTEXPR consists of two expressions sepa-\n"
         "rated by a colon (:), the outcome will be a 2D histogram of the two values\n"
         "plotted versus each other.\n"
         "\n"
         "If PLOTEXPR is not provided, a standard selection of histograms will be\n"
         "produced instead.\n"
         "\n"
         "If CONDEXPR is provided, only particles for which the provided expression\n"
         "evaluates to true will be considered.\n"
         "\n"
         "Examples:\n"
         "1) Simply create and view standard set of histograms for file:\n"
         "   %s myfile.mcpl\n"
         "2) Same, but only for high energy gammas and neutrons:\n"
         "   %s myfile.mcpl where \"(pdgcode==22||pdgcode==2112)&&ekin>1MeV\"\n"
         "3) Get distribution of particle position distance to z-axis in mm:\n"
         "   %s myfile.mcpl \"sqrt(x^2+y^2)/mm\"\n"
         "4) Same, but only for high-energy particles:\n"
         "   %s myfile.mcpl \"sqrt(x^2+y^2)/mm\" where \"ekin>2MeV\"\n"
         "5) Get 2D distribution of particle positions in the x-y plane in cm:\n"
         "   %s myfile.mcpl \"x/cm:y/cm\"\n"
         "",pn,pn,pn,pn,pn,pn);
  return 0;
}


int main(int argc, char** argv) {
  /////////////////
  // Parse input //
  /////////////////

  std::list<std::string> args(argv+1, argv+argc);
  if ( std::find(args.begin(), args.end(), "-h") != args.end()
       || std::find(args.begin(), args.end(), "--help") != args.end() )
    return usage(argv);

  bool opt_nographics = false;
  unsigned long long opt_limit = 0;
  std::string opt_plotexpr;
  std::string opt_condexpr;
  std::string opt_filename;

  if ( std::find(args.begin(), args.end(), "-n") != args.end() ) {
    opt_nographics = true;
    args.erase(std::remove(args.begin(), args.end(), "-n"), args.end());
  }

  if (args.empty())
    return usage(argv,"missing filename");
  opt_filename = args.front();
  args.pop_front();

  std::vector<std::string> fn_parts;
  Core::split(fn_parts,opt_filename,":");

  if (fn_parts.size()>1) {
    if (fn_parts.size()!=2)
      return usage(argv,"bad MCPLFILE argument");
    opt_filename=fn_parts.front();
    std::size_t used(0);
    //extra check for pure digits since stoull accepts negative numbers!
    if (Core::contains_only(fn_parts.back(), "0123456789")) {
      try {
        opt_limit = stoull(fn_parts.back(),&used);
      } catch (std::logic_error&) {
        used=0;
      }
    }
    if (!used||used<fn_parts.back().size())
      return usage(argv,"badly formatted particle load limit");
  }

  if (!Core::file_exists(opt_filename.c_str()))
    return usage(argv,"file not found");

  if (!Core::file_exists_and_readable(opt_filename.c_str()))
    return usage(argv,"file not readable");

  //All that remains is to parse "[PLOTEXPR] [where CONDEXPR]"
  if (!args.empty()) {
    if (args.size()==1) {
      opt_plotexpr = args.front();
    }
    else if (args.size()==2) {
      if (args.front()!="where")
        return usage(argv,"bad syntax");
      opt_condexpr = args.back();
    }
    else if (args.size()==3) {
      opt_plotexpr = args.front();
      args.pop_front();
      if (args.front()!="where")
        return usage(argv,"bad syntax");
      opt_condexpr = args.back();
    } else {
      return usage(argv,"too many arguments");
    }
  }
  if (opt_plotexpr=="where"||opt_condexpr=="where")
    return usage(argv,"bad syntax");

  ///////////////////
  // Do something! //
  ///////////////////

  SimpleHists::HistCollection hc;
  if (!opt_plotexpr.empty()) {
    auto hist = MCPLExtra::mcplHistsFromExpression(opt_filename, opt_plotexpr, opt_condexpr, opt_limit);
    hc.add(hist,"custom");
  } else {
    MCPLExtra::mcplStdHists( hc, opt_filename, opt_condexpr, opt_limit);
  }

#define MCPLEXTRA_OUTFILE_NAME "mcpl.shist"
  const std::string outfile = MCPLEXTRA_OUTFILE_NAME;
  hc.saveToFile(outfile,true);
  printf("created requested histogram%s in %s\n",(opt_plotexpr.empty()?"s":""),outfile.c_str());

  if (!opt_nographics) {
#ifdef MCPLEXTRA_AVOID_USING_PYTHON
    const char * cmd;
    if (opt_plotexpr.empty()) {
      //bring up browser for file:
      cmd = "sb_simplehists_browse " MCPLEXTRA_OUTFILE_NAME;
    } else {
      //just display the single custom histogram:
      cmd = "sb_simplehists_browse -p " MCPLEXTRA_OUTFILE_NAME " custom";
    }
    std::cout<<"Launching: "<<cmd<<std::endl;
    int status = std::system(cmd);
    if ( ! (status >=0 && WIFEXITED( status ) && WEXITSTATUS( status ) == 0 ) )
      throw std::runtime_error(std::string("cmd failed: ")+cmd);
#else
    try {
      pyextra::ensurePyInit();
      if (opt_plotexpr.empty()) {
        //bring up browser for file:
        py::object mod = pyextra::pyimport("SimpleHists.browser");
        mod.attr("interactive_browser")(outfile);
      } else {
        //just display the single custom histogram:
        py::object mod = pyextra::pyimport("SimpleHists.browser");
        mod.attr("interactive_plot_hist_from_file")(outfile,"custom");
      }
    } catch (py::error_already_set&) {
      throw;//TODO: better handling in pybind11 than this?
    }
#endif
  }

  return 0;
}
