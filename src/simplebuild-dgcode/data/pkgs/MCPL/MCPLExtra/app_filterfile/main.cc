#include "MCPLExprParser/MCPLASTBuilder.hh"
#include "MCPL/mcpl.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cassert>

int app_usage( char const* const* argv, const char * errmsg ) {
  if (errmsg) {
    printf("ERROR: %s\n\n",errmsg);
    printf("Run with -h or --help for usage information\n");
    return 1;
  }
  const char * progname = std::strrchr(argv[0], '/');
  progname =  progname ? progname + 1 : argv[0];
  printf("Usage:\n\n");
  printf("  %s [options] <input.mcpl> <output.mcpl> [FILTER]\n\n",progname);
  printf("Produce new MCPL file from an existing one, with a subset of the original\n"
         "particles. The new file will contain same setup and meta-data as the original,\n"
         "apart from the addition of a new comment documenting the editing history.\n"
         "\n"
         "Specify a FILTER in order to skip particles not satisfying desired conditions.\n"
         "\n"
         "Options:\n"
         "\n"
         "  -h, --help   : Show this usage information.\n"
         "  -n, --nogzip : Do not attempt to compress output file.\n"
         "  -l<LIMIT>    : Limit the maximum number of particles in the new file.\n"
         "\n"
         "Examples:\n\n"
         );
  printf("  * Select just neutrons with long wavelength:\n");
  printf("    %s infile.mcpl outfile.mcpl \"is_neutron && neutron_wl > 0.5Aa\"\n",progname);
  printf("  * Select first 100 particles from file:\n");
  printf("    %s infile.mcpl outfile.mcpl -l100\n",progname);
  printf("  * Select first 100 gammas from file:\n");
  printf("    %s infile.mcpl outfile.mcpl -l100 \"pdgcode==22\" \n",progname);
  printf("  * Select all particles in file (can be used to convert to latest MCPL format):\n");
  printf("    %s infile.mcpl outfile.mcpl\n",progname);

  return 0;
}

int parse_args(int argc,const char **argv,
               const char** infile, const char **outfile,
               const char **filterexpression,
               std::uint64_t* nparticles_limit, int* do_gzip) {
  //returns: 0 all ok, 1: error, -1: all ok but do nothing (-h/--help mode)
  *infile = 0;
  *outfile = 0;
  *filterexpression = 0;
  *nparticles_limit = 0;
  *do_gzip = 1;

  int64_t opt_num_limit = -1;
  for (int i = 1; i<argc; ++i) {
    const char * a = argv[i];
    size_t n = strlen(a);
    if (!n)
      continue;
    if (n>=2&&a[0]=='-'&&a[1]!='-') {
      //short options:
      int64_t * consume_digit = 0;
      for (size_t j=1; j<n; ++j) {
        if (consume_digit) {
          if (a[j]<'0'||a[j]>'9')
            return app_usage(argv,"Bad option: expected number");
          *consume_digit *= 10;
          *consume_digit += a[j] - '0';
          continue;
        }
        switch(a[j]) {
        case 'h': app_usage(argv,0); return -1;
        case 'n': *do_gzip = 0; break;
        case 'l': consume_digit = &opt_num_limit; break;
        default:
          return app_usage(argv,"Unrecognised option");
        }
        if (consume_digit) {
          *consume_digit = 0;
          if (j+1==n)
            return app_usage(argv,"Bad option: missing number");
        }
      }
    } else if (n==6 && strcmp(a,"--help")==0) {
      app_usage(argv,0);
      return -1;
    } else if (n==8 && strcmp(a,"--nogzip")==0) {
      *do_gzip = 0;
    } else if (n>=1&&a[0]!='-') {
      if (*filterexpression)
        return app_usage(argv,"Too many arguments.");
      if (*outfile) *filterexpression = a;
      else if (*infile) *outfile = a;
      else *infile = a;
    } else {
      return app_usage(argv,"Bad arguments");
    }
  }

  if (!*infile)
    return app_usage(argv,"Missing argument : input MCPL file");
  if (!*outfile)
    return app_usage(argv,"Missing argument : output MCPL file");
  if (!*filterexpression)
    *filterexpression = "(true)";

  if (opt_num_limit<=0)
    opt_num_limit = 0;
  *nparticles_limit = opt_num_limit;

  return 0;
}

int main(int argc, char** argv) {
  const char * infile;
  const char * outfile;
  const char * filterexpression;
  std::uint64_t nparticles_limit;
  int do_gzip;
  int parse = parse_args( argc, (const char**)argv,
                          &infile, &outfile, &filterexpression,
                          &nparticles_limit, &do_gzip );
  if (parse==-1)// --help
    return 0;
  if (parse)// parse error
    return parse;

  //Prepare filter:
  MCPLExprParser::MCPLASTBuilder filter_builder;
  ExprParser::Evaluator<bool> eval_filter;
  try {
    eval_filter = filter_builder.createEvaluator<bool>(filterexpression);
  } catch (ExprParser::InputError& e) {
    printf("%s in filter expression : %s\n",e.epType(),e.epWhat());
    eval_filter.setArg(0);
  }
  if (!eval_filter.arg())
    return 1;
  if (eval_filter.isConstant()&&!eval_filter()) {
    printf("WARNING: FILTER expression will always evaluate as false.\n");
    return 1;
  }

  //Prepare MCPL files:
  mcpl_file_t fi = mcpl_open_file(infile);
  mcpl_outfile_t fo = mcpl_create_outfile(outfile);
  mcpl_transfer_metadata(fi, fo);

  //Document the process as a comment:
  std::ostringstream s;
  const char * progname = std::strrchr(argv[0], '/');
  progname =  progname ? progname + 1 : argv[0];
  s << "File edited by "<<progname;
  if (nparticles_limit)
    s << " (nlimit="<<nparticles_limit<<", ";
  else
    s << " (";
  if (eval_filter.isConstant()&&eval_filter())
    s << "<unfiltered>, ";
  else
    s <<"filter='"<<filterexpression<<"', ";
  s << "norig="<<mcpl_hdr_nparticles(fi)<<")";
  mcpl_hdr_add_comment(fo,s.str().c_str());

  //Transfer particles:
  std::uint64_t used(0), posp1(0);
  double norig = mcpl_hdr_nparticles(fi);
  int progress = -1;
  const mcpl_particle_t* p;
  printf("Start processing particle data.\n");
  while ( ( p=mcpl_read(fi) ) ) {
    ++posp1;
    if (posp1%500000==0) {
      int prog = int(0.5+mcpl_currentposition(fi)*100.0/norig);
      if (prog!=progress) {
        printf("%4i %% of file processed\n",prog);
        progress = prog;
      }
    }
    filter_builder.setCurrentParticle(p);
    if ( !eval_filter() )
      continue;
    mcpl_add_particle(fo,p);
    ++used;
    if ( nparticles_limit && used==nparticles_limit )
      break;
  }
  printf("Done processing particle data.\n");

  //close:
  if (do_gzip)
    mcpl_closeandgzip_outfile(fo);
  else
    mcpl_close_outfile(fo);
  mcpl_close_file(fi);

  return 0;
}
