#include "ExprParser/Types.hh"
#include "MCPL/mcpl.h"
#include "Utils/NeutronMath.hh"
#include "Units/Units.hh"

namespace MCPLExprParser {

  namespace DataExtractors {

    using ExprParser::int_type;
    using ExprParser::float_type;

    inline float_type ekin(const mcpl_particle_t* p) { static_assert(Units::MeV==1.0,""); return p->ekin; }
    inline float_type polx(const mcpl_particle_t* p) { return p->polarisation[0]; }
    inline float_type poly(const mcpl_particle_t* p) { return p->polarisation[1]; }
    inline float_type polz(const mcpl_particle_t* p) { return p->polarisation[2]; }
    inline float_type x(const mcpl_particle_t* p) { return p->position[0] * Units::cm; }
    inline float_type y(const mcpl_particle_t* p) { return p->position[1] * Units::cm; }
    inline float_type z(const mcpl_particle_t* p) { return p->position[2] * Units::cm; }
    inline float_type posx(const mcpl_particle_t* p) { return p->position[0] * Units::cm; }
    inline float_type posy(const mcpl_particle_t* p) { return p->position[1] * Units::cm; }
    inline float_type posz(const mcpl_particle_t* p) { return p->position[2] * Units::cm; }
    inline float_type dirx(const mcpl_particle_t* p) { return p->direction[0]; }
    inline float_type diry(const mcpl_particle_t* p) { return p->direction[1]; }
    inline float_type dirz(const mcpl_particle_t* p) { return p->direction[2]; }
    inline float_type ux(const mcpl_particle_t* p) { return p->direction[0]; }
    inline float_type uy(const mcpl_particle_t* p) { return p->direction[1]; }
    inline float_type uz(const mcpl_particle_t* p) { return p->direction[2]; }
    inline float_type time(const mcpl_particle_t* p) { return p->time * Units::millisecond; }
    inline float_type weight(const mcpl_particle_t* p) { return p->weight; }
    inline int_type pdgcode(const mcpl_particle_t* p) { return p->pdgcode; }
    inline int_type is_neutron(const mcpl_particle_t* p) { return p->pdgcode==2112; }
    inline int_type is_gamma(const mcpl_particle_t* p) { return p->pdgcode==22; }
    inline int_type is_photon(const mcpl_particle_t* p) { return p->pdgcode==22; }
    inline int_type is_neutrino(const mcpl_particle_t* p)
    {
      auto pp = std::abs(p->pdgcode);
      return pp==12||pp==14||pp==16;
    }
    inline int_type is_ion(const mcpl_particle_t* p)
    {
      auto pp = std::abs(p->pdgcode);
      return pp/100000000 == 10;
    }
    inline int_type userflags(const mcpl_particle_t* p) { return p->userflags; }
    inline float_type neutron_wl(const mcpl_particle_t* p) {
      if (p->pdgcode!=2112)
        EXPRPARSER_THROW(DomainError,"neutron_wl must only be called for neutrons"
                         " (use \"is_neutron\" in your expression to make sure,"
                         " like \"is_neutron && neutron_wl > 2Aa\")");
      static_assert(Units::MeV==1.0,"");
      return Utils::neutronEKinToWavelength(p->ekin);
    }

  }

}
