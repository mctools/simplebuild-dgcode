#include "MCPLExtra/HistCreate.hh"
#include "MCPLExprParser/MCPLASTBuilder.hh"
#include "MCPL/mcpl.h"
#include <sstream>
#include <iomanip>
#include "Utils/NeutronMath.hh"
#include "Core/String.hh"

//Todo: 50th(?) pdg code seen and above will collapse into "other" - same plot for userflags?
//Todo: No need to use expensive histogram filling on first run through data when all we need is the range of data.

namespace MCPLExtra {

  std::string mini_pdg_database(long pdgcode)
  {
    switch(pdgcode) {
    case 12: return "nu_e";
    case 14: return "nu_mu";
    case 16: return "nu_tau";
    case -12: return "nu_e-bar";
    case -14: return "nu_mu-bar";
    case -16: return "nu_tau-bar";
    case 2112: return "n";
    case 2212: return "p";
    case -2112: return "n-bar";
    case -2212: return "p-bar";
    case 22: return "gamma";
    case 11: return "e-";
    case -11: return "e+";
    case 13: return "mu-";
    case -13: return "mu+";
    case 15: return "tau-";
    case -15: return "tau+";
    case 211: return "pi+";
    case -211: return "pi-";
    case 111: return "pi0";
    case 321: return "K+";
    case -321: return "K-";
    case 130: return "Klong";
    case 310: return "Kshort";
    case -1000010020: return "D-bar";
    case -1000010030: return "T-bar";
    case 1000010020: return "D";
    case 1000010030: return "T";
    case 1000020040: return "alpha";
    case -1000020040: return "alpha-bar";
    }
    static const char* s_elementsSymbols[] = {"H",  "He", "Li", "Be", "B",  "C",  "N",  "O",  "F",  "Ne",
                                              "Na", "Mg", "Al", "Si", "P" , "S",  "Cl", "Ar", "K",  "Ca", "Sc",
                                              "Ti", "V",  "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn", "Ga", "Ge",
                                              "As", "Se", "Br", "Kr", "Rb", "Sr", "Y",  "Zr", "Nb", "Mo", "Tc",
                                              "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn", "Sb", "Te", "I",  "Xe",
                                              "Cs", "Ba", "La", "Ce", "Pr", "Nd", "Pm", "Sm", "Eu", "Gd", "Tb",
                                              "Dy", "Ho", "Er", "Tm", "Yb", "Lu", "Hf", "Ta", "W",  "Re", "Os",
                                              "Ir", "Pt", "Au", "Hg", "Tl", "Pb", "Bi", "Po", "At", "Rn", "Fr",
                                              "Ra", "Ac", "Th", "Pa", "U",  "Np", "Pu", "Am", "Cm", "Bk", "Cf",
                                              "Es", "Fm", "Md", "No", "Lr", "Rf", "Db", "Sg", "Bh", "Hs", "Mt",
                                              "Ds", "Rg"};

    if (pdgcode>0&&pdgcode/100000000==10) {
      long tmp = pdgcode;
      unsigned I = tmp % 10;
      tmp /= 10;
      unsigned AAA = tmp%1000;
      tmp /= 1000;
      unsigned ZZZ = tmp%1000;
      tmp /= 1000;
      unsigned L = tmp % 10;
      tmp /= 10;
      if (tmp==10&&ZZZ>0&&AAA>0) {
        std::ostringstream s;
        if (L==0&&I==0&&ZZZ<sizeof(s_elementsSymbols)/sizeof(char*)+1) {
          s<<s_elementsSymbols[ZZZ-1]<<AAA;
          return s.str();
        }
        s << "ion(Z="<<ZZZ<<",A="<<AAA;
        if (L)
          s<<",L="<<L;
        if (I)
          s<<",I="<<I;
        s<<")";
        return s.str();
      }
    }
    return std::string();
  }

  unsigned suggest_nbins(unsigned long long nfills)
  {
    if (nfills<1000)
      return 100;
    return 200;
  }

  void adjust_limits(double& low, double& high) {
    //input data range, output suggested limits for data histogram
    if (high<=low) {
      if (low==0) {
        low=-0.5; high=0.5;
      } else {
        double mag = low<0 ? -low*0.1 : low*0.1;
        high = low+mag;
        low = low-mag;
      }
    } else {
      double eps = (high-low)*1.0e-5;
      bool is_pos = (low>0);
      bool is_neg = (high<0);
      low -= eps;
      high += eps;
      if (is_pos&&low<0) low = 0;
      if (is_neg&&high>0) high = 0;
    }
  }

  void adjust_booking(unsigned long long nfills, SimpleHists::Hist1D*h) {
    double a(0.0),b(0.0);
    if (!h->empty()) {
      a = h->getMinFilled();
      b = h->getMaxFilled();
    }
    adjust_limits(a,b);
    h->resetAndRebin(suggest_nbins(nfills), a,b);
  }

  ExprParser::Evaluator<bool> prepareFilter(MCPLExprParser::MCPLASTBuilder& builder,
                                            const std::string& filter_expr)
  {
    auto eval_filter = builder.createEvaluator<bool>(filter_expr.empty()?"true":filter_expr);
    if (eval_filter.isConstant()) {
      if (eval_filter()) {
        eval_filter.setArg(0);//disable
      } else {
        throw std::runtime_error("Filter always evaluates to false - histograms would be empty");
      }
    }
    return eval_filter;
  }
}

void MCPLExtra::mcplStdHists( SimpleHists::HistCollection& hc,
                              const std::string& filename,
                              const std::string& filter_expr,
                              unsigned long long max_particles_load )
{
  //Prepare filter:
  MCPLExprParser::MCPLASTBuilder builder;
  auto eval_filter = prepareFilter(builder,filter_expr);
  bool do_filter = (bool)eval_filter.arg();

  //Open file:
  mcpl_file_t f = mcpl_open_file(filename.c_str());
  if (max_particles_load==0)
    max_particles_load = mcpl_hdr_nparticles(f);
  unsigned long long left = max_particles_load;
  const bool has_userflags = mcpl_hdr_has_userflags(f);
  const bool has_polarisation = mcpl_hdr_has_polarisation(f);

  auto h_ekin = hc.book1D("ekin [MeV]", 1, 0.0, 1.0, "ekin");
  auto h_time = hc.book1D("time [ms]", 1, 0.0, 1.0, "time");
  auto h_posx = hc.book1D("X-coordinate of position [cm]", 1, 0.0, 1.0, "posx");
  auto h_posy = hc.book1D("Y-coordinate of position [cm]", 1, 0.0, 1.0, "posy");
  auto h_posz = hc.book1D("Z-coordinate of position [cm]", 1, 0.0, 1.0, "posz");
  auto h_dirx = hc.book1D("X-coordinate of direction", 1, 0.0, 1.0, "dirx");
  auto h_diry = hc.book1D("Y-coordinate of direction", 1, 0.0, 1.0, "diry");
  auto h_dirz = hc.book1D("Z-coordinate of direction", 1, 0.0, 1.0, "dirz");
  SimpleHists::Hist1D * h_polx(0);
  SimpleHists::Hist1D * h_poly(0);
  SimpleHists::Hist1D * h_polz(0);
  SimpleHists::Hist1D * h_uf(0);
  if (has_polarisation) {
    h_polx = hc.book1D("X-component of polarisation vector", 1, 0.0, 1.0, "polx");
    h_poly = hc.book1D("Y-component of polarisation vector", 1, 0.0, 1.0, "poly");
    h_polz = hc.book1D("Z-component of polarisation vector", 1, 0.0, 1.0, "polz");
  }
  if (has_userflags) {
    //often not very useful to plot these, but at least it reminds the user that userflags exists.
    h_uf = hc.book1D("Userflags (raw integer value)", 1, 0.0, 1.0, "userflags");
  }
  auto h_weight = hc.book1D("weight", 1, 0.0, 1.0, "weight");
  auto h_nwl = hc.book1D("Neutron wavelengths [Aa]", 1, 0.0, 1.0, "neutron_wl");
  auto h_pdgcode = hc.bookCounts("Particle Type [PDG codes]", "pdgcode");

  //Using map, could use sorted vector instead for faster access:
  std::map<long,SimpleHists::HistCounts::Counter> pdgcounters;

  //Loop through file to collect stats data for limits (despite the overhead, we
  //do it by filling the hists themselves to take advantage of the statistics
  //calculations there):
  unsigned long long nused(0);
  unsigned long long nneutrons(0);
  const mcpl_particle_t* p;
  while ( left && (p=mcpl_read(f)) ) {
    --left;
    if ( do_filter ) {
      builder.setCurrentParticle(p);
      if ( !eval_filter() )
        continue;
    }
    ++nused;
    h_ekin->fill(p->ekin,p->weight);
    h_posx->fill(p->position[0],p->weight);
    h_posy->fill(p->position[1],p->weight);
    h_posz->fill(p->position[2],p->weight);
    h_dirx->fill(p->direction[0],p->weight);
    h_diry->fill(p->direction[1],p->weight);
    h_dirz->fill(p->direction[2],p->weight);
    if (has_polarisation) {
      h_polx->fill(p->polarisation[0],p->weight);
      h_poly->fill(p->polarisation[1],p->weight);
      h_polz->fill(p->polarisation[2],p->weight);
    }
    if (has_userflags)
      h_uf->fill(p->userflags,p->weight);

    int pdgcode = p->pdgcode;
    auto pdgctr = pdgcounters.find(pdgcode);
    if (pdgctr==pdgcounters.end()) {
      //choose access labels which sorts correctly and contains no forbidden characters
      std::ostringstream label;
      label<< "pdg_"<<(pdgcode<0?"minus":"plus")<<std::setfill('0') << std::setw(10)<<(pdgcode<0?-pdgcode:pdgcode);
      pdgcounters[ pdgcode ] = h_pdgcode->addCounter(label.str());
      pdgctr = pdgcounters.find(pdgcode);
      std::string pname = mini_pdg_database(pdgcode);
      std::ostringstream displaylabel;
      if (pname.empty()) {
        displaylabel << pdgcode;
      } else {
        displaylabel << pname;
      }
      pdgctr->second.setDisplayLabel(displaylabel.str());
    }
    pdgctr->second += p->weight;

    if (pdgcode==2112) {
      ++nneutrons;
      h_nwl->fill(Utils::neutronEKinToWavelength(p->ekin)/Units::angstrom,p->weight);
    }
    h_time->fill(p->time,p->weight);
    h_weight->fill(p->weight);

  }

  //Now, use stats to perform final booking:
  adjust_booking(nused,h_ekin);
  adjust_booking(nused,h_posx);
  adjust_booking(nused,h_posy);
  adjust_booking(nused,h_posz);
  adjust_booking(nused,h_dirx);
  adjust_booking(nused,h_diry);
  adjust_booking(nused,h_dirz);
  if (has_polarisation) {
    adjust_booking(nused,h_polx);
    adjust_booking(nused,h_poly);
    adjust_booking(nused,h_polz);
  }
  if (has_userflags)
    adjust_booking(nused,h_uf);
  adjust_booking(nused,h_time);
  adjust_booking(nused,h_weight);

  double a = h_nwl->empty() ? 0.0 : h_nwl->getMinFilled();
  double b = h_nwl->empty() ? 0.0 : h_nwl->getMaxFilled();
  if (b>20)
    b=20.0;
  adjust_limits(a,b);
  h_nwl->resetAndRebin(suggest_nbins(nneutrons), a,b);

  //rewind:
  left = max_particles_load;
  mcpl_rewind(f);

  //fill again (fixme: Code copied from above, a bit sloppy!):
  while ( left && (p=mcpl_read(f)) ) {
    --left;
    if ( do_filter ) {
      builder.setCurrentParticle(p);
      if ( !eval_filter() )
        continue;
    }
    h_ekin->fill(p->ekin,p->weight);
    h_posx->fill(p->position[0],p->weight);
    h_posy->fill(p->position[1],p->weight);
    h_posz->fill(p->position[2],p->weight);
    h_dirx->fill(p->direction[0],p->weight);
    h_diry->fill(p->direction[1],p->weight);
    h_dirz->fill(p->direction[2],p->weight);
    if (has_polarisation) {
      h_polx->fill(p->polarisation[0],p->weight);
      h_poly->fill(p->polarisation[1],p->weight);
      h_polz->fill(p->polarisation[2],p->weight);
    }
    if (has_userflags)
      h_uf->fill(p->userflags,p->weight);
    if (p->pdgcode==2112)
      h_nwl->fill(Utils::neutronEKinToWavelength(p->ekin)/Units::angstrom,p->weight);
    h_time->fill(p->time,p->weight);
    h_weight->fill(p->weight);
  }

  h_pdgcode->sortByLabels();
}

SimpleHists::HistBase * MCPLExtra::mcplHistsFromExpression(const std::string& filename,
                                                           const std::string& plot_expr,
                                                           const std::string& filter_expr,
                                                           unsigned long long max_particles_load )
{
  //Prepare plot expression:
  MCPLExprParser::MCPLASTBuilder builder;
  std::vector<std::string> parts;
  ExprParser::Evaluator<ExprParser::float_type> expr1, expr2;
  Core::split(parts,plot_expr,":");
  bool twodim(false);
  if (parts.size()==2) {
    twodim = true;
    expr1 = builder.createEvaluator<ExprParser::float_type>(parts.front());
    expr2 = builder.createEvaluator<ExprParser::float_type>(parts.back());
  } else if (parts.size()==1) {
    expr1 = builder.createEvaluator<ExprParser::float_type>(plot_expr);
  } else {
    EXPRPARSER_THROW(ParseError,"invalid plot expression - must be of form \"<expr>\" or \"<expr1>:<expr2>\"");
    return 0;
  }

  //Prepare filter:
  auto eval_filter = prepareFilter(builder,filter_expr);
  bool do_filter = (bool)eval_filter.arg();

  //Open file:
  mcpl_file_t f = mcpl_open_file(filename.c_str());
  if (max_particles_load==0)
    max_particles_load = mcpl_hdr_nparticles(f);
  unsigned long long left = max_particles_load;

  //First loop - collect limits
  double expr1_min(0.0), expr1_max(0.0);
  double expr2_min(0.0), expr2_max(0.0);
  bool first(true);
  const mcpl_particle_t* p;
  while ( left && (p=mcpl_read(f)) ) {
    --left;
    builder.setCurrentParticle(p);
    if ( do_filter ) {
      if ( !eval_filter() )
        continue;
    }
    if (first) {
      first=false;
      expr1_min = expr1_max = expr1();
      if (twodim)
        expr2_min = expr2_max = expr2();
    } else {
      auto v1 = expr1();
      if (v1<expr1_min) expr1_min=v1;
      if (v1>expr1_max) expr1_max=v1;
      if (twodim) {
        auto v2 = expr2();
        if (v2<expr2_min) expr2_min=v2;
        if (v2>expr2_max) expr2_max=v2;
      }
    }
  }

  //Create histogram:
  SimpleHists::Hist1D * h1(0);
  SimpleHists::Hist2D * h2(0);
  if (expr1_min>=expr1_max) {
    expr1_max = expr1_min + 0.5;
    expr1_min = expr1_min - 0.5;
  }
  if (expr2_min>=expr2_max) {
    expr2_max = expr2_min + 0.5;
    expr2_min = expr2_min - 0.5;
  }
  if (twodim) {
    h2 = new SimpleHists::Hist2D(200, expr1_min, expr1_max,200, expr2_min, expr2_max);
    h2->setXLabel(parts.front());
    h2->setYLabel(parts.back());
  } else {
    h1 = new SimpleHists::Hist1D(plot_expr, 200, expr1_min, expr1_max);
  }

  //rewind:
  left = max_particles_load;
  mcpl_rewind(f);

  //Finally, fill data:
  while ( left && (p=mcpl_read(f)) ) {
    --left;
    builder.setCurrentParticle(p);
    if ( do_filter ) {
      if ( !eval_filter() )
        continue;
    }
    if (twodim)
      h2->fill(expr1(),expr2(),p->weight);
    else
      h1->fill(expr1(),p->weight);
  }

  if (h2)
    return h2;
  else
    return h1;
}

