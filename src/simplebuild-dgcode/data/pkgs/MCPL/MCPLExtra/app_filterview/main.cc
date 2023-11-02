#include "MCPL/mcpl.h"
#include "MCPLExprParser/MCPLASTBuilder.hh"
#include "ExprParser/ASTDebug.hh"
#include "Core/File.hh"
#include <algorithm>
#include <cstring>

//Access hidden function from mcpl.c:
extern "C"{
  void mcpl_dump_particles(mcpl_file_t f, std::uint64_t nskip, std::uint64_t nlimit,
                           int(filter)(const mcpl_particle_t*));
}

int usage(char ** argv, const char * errmsg = 0)
{
  if (errmsg)
    printf("\nError: %s\n\n",errmsg);

  const char * pn = std::strrchr(argv[0], '/');
  pn = pn ? pn + 1 : argv[0];

  printf("Usage:\n\n");
  printf("%s EXPR [MCPLFILE]\n\n",pn);
  printf("Applies the filter EXPR to the particles in MCPLFILE and\n");
  printf("lists the selected particles thus selected. If no MCPLFILE\n");
  printf("is provided, the compiled abstract-syntax-tree representa-\n");
  printf("tion of EXPR is shown.\n\n");
  printf("Examples:\n\n");
  printf("  %s \"is_neutron && neutron_wl <= 1.8Aa\" myfile.mcpl.gz\n",pn);
  printf("  %s \"is_neutron && inrange(neutron_wl,1.1Aa,1.8Aa)\" myfile.mcpl.gz\n",pn);
  printf("  %s \"sqrt(x^2+y^2) < 10cm\" myfile.mcpl.gz\n",pn);

  return errmsg ? 1 : 0;
}

namespace MCPLFilterFunc {
  static MCPLExprParser::MCPLASTBuilder builder;
  static ExprParser::Evaluator<bool> eval_filter;
  static unsigned long nselected = 0;
  static unsigned long ntested = 0;
  int filterfunc(const mcpl_particle_t * p) {
    ++ntested;
    builder.setCurrentParticle(p);
    if (eval_filter()) {
      ++nselected;
      return 1;
    }
    return 0;
  }
}

int main(int argc,char** argv) {

  std::vector<std::string> args(argv+1, argv+argc);

  if ( std::find(args.begin(), args.end(), "-h") != args.end()
       || std::find(args.begin(), args.end(), "--help") != args.end() )
    return usage(argv);

  std::string opt_expr;
  std::string opt_file;

  if (args.size()==2) {
    opt_expr = args[0];
    opt_file = args[1];
  } else if (args.size()==1) {
    opt_expr = args[0];
  } else {
    return usage(argv,"wrong number of arguments");
  }

  if (!opt_file.empty()) {
    if (!Core::file_exists(opt_file))
      return usage(argv,"file not found");
    if (Core::isdir(opt_file))
      return usage(argv,"specified file is actually a directory");
    if (!Core::file_exists_and_readable(opt_file))
      return usage(argv,"file not readable");
  }

  try {
    MCPLFilterFunc::eval_filter = MCPLFilterFunc::builder.createEvaluator<bool>(opt_expr);
  } catch (ExprParser::InputError& e) {
    printf("%s in filter expression : %s\n",e.epType(),e.epWhat());
    MCPLFilterFunc::eval_filter.setArg(0);
  }

  if (!MCPLFilterFunc::eval_filter.arg())
    return 1;


  if (opt_file.empty()) {
    printf("\nfilter expression: \"%s\"\n\n",opt_expr.c_str());
    printf("compiles into the following abstract syntax tree representation:\n\n");
    ExprParser::printTree(MCPLFilterFunc::eval_filter.arg(),"                   |",false);
    printf("\n");
  } else {
    //using default mcpl error handler which prints error messages and terminates
    //the process in case of errors.
    mcpl_file_t mcplfile = mcpl_open_file(opt_file.c_str());

    mcpl_dump_particles(mcplfile, 0, 0, MCPLFilterFunc::filterfunc);
    printf("Filter \"%s\" selected %lu/%lu particles (%g %%) from the file.\n",
           opt_expr.c_str(),
           MCPLFilterFunc::nselected,
           MCPLFilterFunc::ntested,
           MCPLFilterFunc::nselected*100.0/MCPLFilterFunc::ntested);
  }

  return 0;
}
